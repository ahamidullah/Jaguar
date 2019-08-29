#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include <dirent.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include "../jaguar.h"

const char *output_directory = "code/generated";

typedef struct {
	DIR           *dir;
	struct dirent *dirent;
	char          *filename;
	u8             is_directory;
} Directory_Iteration;

void _abort_actual(const char *file_name, s32 line, const char *function_name, const char *format, ...) {
	va_list arguments;
	va_start(arguments, format);
	printf("\n###########################################################################\n");
	printf("[PROGRAM ABORT]\n");
	printf("%s: %s: %d:\n", file_name, function_name, line);
	vprintf(format, arguments);
	printf("\n");
	printf("###########################################################################\n\n");
	va_end(arguments);
	assert(0);
	exit(1);
}

u8 iterate_through_all_files_in_directory(const char *path, Directory_Iteration *context) {
	if (!context->dir) { // First read.
		context->dir = opendir(path);
		if (!context->dir) {
			printf("Failed to open directory %s: %s\n", path, strerror(errno));
			return 0;
		}
	}
	while ((context->dirent = readdir(context->dir))) {
		if (!strcmp(context->dirent->d_name, ".") || !strcmp(context->dirent->d_name, "..")) {
			continue;
		}
		context->filename = context->dirent->d_name;
		context->is_directory = (context->dirent->d_type == DT_DIR);
		return 1;
	}
	return 0;
}

const char *find_last_occurrence_of_character(const char *string, char character) {
	const char *occurrence = NULL;
	while (*string) {
		if (*string == character) {
			occurrence = string;
		}
		string++;
	}
	return occurrence;
}

void read_entire_file(const char *path, char *output) {
	FILE *file_handle = fopen(path, "r");
	if (!file_handle) {
		_abort("Failed to open file %s: %s\n", path, strerror(errno));
	}
	fseek(file_handle, 0L, SEEK_END);
	s64 file_length = ftell(file_handle);
	fseek(file_handle, 0L, SEEK_SET);
	size_t read_byte_count = fread(output, 1, file_length, file_handle);
	if (read_byte_count != file_length) {
		_abort("Failed to read file %s: %s\n", path, strerror(errno));
	}
	output[file_length] = '\0';
	fclose(file_handle);
}

void join_filepaths(const char *a, const char *b, char *output) {
	size_t a_length = strlen(a);
	size_t b_length = strlen(b);
	strcpy(output, a);
	output[a_length] = '/';
	strcpy(output + a_length + 1, b);
	output[a_length + b_length + 1] = '\0';
}

const char *glsl_to_jaguar_type(const char *glsl_type_name) {
	if (!strcmp(glsl_type_name, "vec4")) {
		return "V4";
	}
	if (!strcmp(glsl_type_name, "mat4")) {
		return "M4";
	}
	return "Unknown_Type";
}

#define MAX_TOKEN_LENGTH 256

u32 shader_line_number = 0;
u32 shader_character_offset = 0;
char *shader_mark = NULL;
char *shader_token_delimiters = " \t;,(){}=-+\n";

void eat_spaces() {
	while(*shader_mark == ' ') {
		shader_mark++;
	}
}

u8 eat_until_character(char character) {
	while (*shader_mark && *shader_mark != character) {
		shader_mark++;
	}
	if (!*shader_mark) {
		return 0;
	}
	return 1;
}

u8 eat_past_character(char character) {
	while (*shader_mark && *shader_mark != character) {
		shader_mark++;
	}
	if (!*shader_mark) {
		return 0;
	}
	*shader_mark++;
	return 1;
}

u8 eat_until_one_of(const char *characters) {
	u32 delimiter_count = strlen(characters);
	while (*shader_mark) {
		for (u32 i = 0; i < delimiter_count; i++) {
			if (*shader_mark == characters[i]) {
				return 1;
			}
		}
		shader_mark++;
	}
	return 0;
}

u8 is_delimiter(char character) {
	for (u32 i = 0; i < strlen(shader_token_delimiters); i++) {
		if (character == shader_token_delimiters[i]) {
			return 1;
		}
	}
	return 0;
}

u8 get_token(char *token) {
	char *token_start = token;
	while (*shader_mark && (isspace(*shader_mark))) {
		shader_mark++;
	}
	u32 token_length = 0;
	while (*shader_mark) {
		if (is_delimiter(*shader_mark)) {
			if (token_length == 0) {
				*token++ = *shader_mark++;
				token_length++;
			}
			break;
		}
		*token++ = *shader_mark++;
		token_length++;
		assert(token_length < MAX_TOKEN_LENGTH);
	}
	*token++ = '\0';
	return token_length > 0;
}

u8 get_expected_token(char *token, const char *expected) {
	if (!get_token(token)) {
		return 0;
	}
	if (strcmp(token, expected)) {
		_abort("Invalid token at %u:%u: expected:'%s', got:'%s'.\n", shader_line_number, shader_character_offset, expected, token);
	}
	return 1;
}

// parse_value parses the value in ' = x'.
u32 parse_value() {
	char token[MAX_TOKEN_LENGTH];
	get_expected_token(token, "=");
	get_token(token);
	return atoi(token);
}

#define MAX_NAME_LENGTH        64
#define MAX_FIELDS_PER_UNIFORM 16
#define MAX_UNIFORMS           256

typedef struct {
	char type[MAX_NAME_LENGTH];
	char name[MAX_NAME_LENGTH];
} Uniform_Field;

typedef struct {
	char          name[MAX_NAME_LENGTH];
	u32           set;
	u32           binding;
	u8            is_push_constant;
	u32           field_count;
	Uniform_Field fields[MAX_FIELDS_PER_UNIFORM];
} Uniform;

Uniform uniforms[MAX_UNIFORMS] = {};
u32 uniform_count = 0;

void process_shaders() {
	const char *shader_directory = "code/shaders";
	Directory_Iteration iteration = {};
	while (iterate_through_all_files_in_directory(shader_directory, &iteration)) {
		const char *file_extension = find_last_occurrence_of_character(iteration.filename, '.');
		if (strcmp(file_extension, ".glsl")) {
			continue;
		}
		if (strcmp(iteration.filename, "flat_color.glsl")) {
			continue;
		}
		static char shader_path[512], file_contents[1 << 6];
		join_filepaths(shader_directory, iteration.filename, shader_path);
		read_entire_file(shader_path, file_contents);

		char shader_name[MAX_NAME_LENGTH];
		strncpy(shader_name, iteration.filename, file_extension - iteration.filename);
		shader_name[file_extension - iteration.filename] = '\0';

		char type_name_prefix[MAX_NAME_LENGTH];
		strcpy(type_name_prefix, shader_name);
		for (u32 i = 0; i < strlen(type_name_prefix) - 1; i++) {
			if (i == 0 || type_name_prefix[i - 1] == '_') {
				type_name_prefix[i] = toupper(type_name_prefix[i]);
			}
		}

		char define_prefix[256];
		strcpy(define_prefix, shader_name);
		for (u32 i = 0; i < strlen(define_prefix); i++) {
			define_prefix[i] = toupper(define_prefix[i]);
		}

		char output_filepath[256];
		join_filepaths(output_directory, "vulkan.h", output_filepath);
		FILE *output_file_handle = fopen(output_filepath, "w");
		fprintf(output_file_handle, "// This file was generated by the preprocessor.\n\n");

		shader_mark = file_contents;
		char token[MAX_TOKEN_LENGTH];
		u32 line_number = 0, character_offset = 0;
		while(get_token(token)) {
			if (!strcmp(token, "layout")) {
				get_expected_token(token, "(");
				while (strcmp(token, ")")) {
					get_token(token);
					if (!strcmp(token, "set")) {
						uniforms[uniform_count].set = parse_value();
					}
					if (!strcmp(token, "binding")) {
						uniforms[uniform_count].binding = parse_value();
					}
					if (!strcmp(token, "push_constant")) {
						uniforms[uniform_count].is_push_constant = 1;
					}
					if (!strcmp(token, "location")) {
						parse_value();
					}
					get_token(token);
				}
				get_token(token);
				if (!strcmp(token, "uniform")) {
					// Parse uniform.
					get_token(uniforms[uniform_count].name);
					//strcpy(uniforms[uniform_count].name, type_name_prefix);
					//strcat(uniforms[uniform_count].name, token);
					get_expected_token(token, "{");
					while (get_token(token) && strcmp(token, "}")) {
						if (!strcmp(token, "layout")) {
							eat_past_character(')');
							get_token(token);
						}
						strcpy(uniforms[uniform_count].fields[uniforms[uniform_count].field_count].type, token);
						get_token(uniforms[uniform_count].fields[uniforms[uniform_count].field_count].name);
						get_expected_token(token, ";");
						uniforms[uniform_count].field_count++;
						assert(uniforms[uniform_count].field_count < MAX_FIELDS_PER_UNIFORM);
					}
					fprintf(output_file_handle, "typedef struct {\n");
					for (u32 j = 0; j < uniforms[uniform_count].field_count; j++) {
						fprintf(output_file_handle, "\t%s %s;\n", glsl_to_jaguar_type(uniforms[uniform_count].fields[j].type), uniforms[uniform_count].fields[j].name);
					}
					fprintf(output_file_handle, "} %s_%s;\n\n", type_name_prefix, uniforms[uniform_count].name);
					char upper_type_name[MAX_NAME_LENGTH];
					for (u32 i = 0; i < strlen(uniforms[uniform_count].name); i++) {
						upper_type_name[i] = toupper(uniforms[uniform_count].name[i]);
					}
					upper_type_name[strlen(uniforms[uniform_count].name)] = '\0';
					if (!uniforms[uniform_count].is_push_constant) {
						fprintf(output_file_handle, "#define %s_%s_DESCRIPTOR_SET_NUMBER %u\n", define_prefix, upper_type_name, uniforms[uniform_count].set);
						fprintf(output_file_handle, "#define %s_%s_BINDING_NUMBER %u\n", define_prefix, upper_type_name, uniforms[uniform_count].binding);
						fprintf(output_file_handle, "\n");
					}
					if (!uniforms[uniform_count].is_push_constant) {
						uniform_count++;
					}
					assert(uniform_count < MAX_UNIFORMS);
				} else if (!strcmp(token, "in")) {
					get_token(token);
					get_token(token);
				}
			}
		}
		fprintf(output_file_handle, "#define %s_DESCRIPTOR_SET_TYPE_COUNT %u\n\n", define_prefix, uniform_count);
		fclose(output_file_handle);
	}
}

s32 main(s32 argc, char **argv) {
	process_shaders();
}
