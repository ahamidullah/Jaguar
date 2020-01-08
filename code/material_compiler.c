// @TODO: Split the material compiler and the shader compiler.
// @TODO: Shouldn't world space be world coordinate system?

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <execinfo.h>
#include <dirent.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef float f32;
typedef double f64;

void Abort(const char *format, ...) {
	va_list arguments;
	va_start(arguments, format);
	printf("###########################################################################\n");
	printf("[PROGRAM ABORT]\n");
	vprintf(format, arguments);
	printf("\n");
	printf("###########################################################################\n");
	va_end(arguments);
	assert(0);
	exit(1);
}

typedef struct Directory_Iteration {
	DIR *dir;
	struct dirent *dirent;
	char *filename;
	bool is_directory;
} Directory_Iteration;

u32 Get_Number_Of_Files_In_Directory(const char *path) {
	DIR *dir = opendir(path);
	assert(dir);
	struct dirent *entry;
	s32 file_count = 0;
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			continue;
		}
		if (entry->d_type == DT_REG || entry->d_type == DT_DIR) {
			file_count++;
		}
	}
	closedir(dir);
	return file_count;
}

bool Iterate_Through_All_Files_In_Directory(const char *path, Directory_Iteration *context) {
	if (!context->dir) { // First read.
		context->dir = opendir(path);
		if (!context->dir) {
			Abort("Failed to open directory %s: %s\n", path, strerror(errno));
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

void Read_Entire_File(const char *path, char *output) {
	FILE *file_handle = fopen(path, "r");
	if (!file_handle) {
		Abort("Failed to open file %s: %s\n", path, strerror(errno));
	}
	fseek(file_handle, 0L, SEEK_END);
	s64 file_length = ftell(file_handle);
	fseek(file_handle, 0L, SEEK_SET);
	size_t read_byte_count = fread(output, 1, file_length, file_handle);
	if (read_byte_count != file_length) {
		Abort("Failed to read file %s: %s\n", path, strerror(errno));
	}
	output[file_length] = '\0';
	fclose(file_handle);
}

void Join_Filepaths(const char *a, const char *b, char *output) {
	size_t a_length = strlen(a);
	size_t b_length = strlen(b);
	strcpy(output, a);
	output[a_length] = '/';
	strcpy(output + a_length + 1, b);
	output[a_length + b_length + 1] = '\0';
}

bool Is_Delimiter(char character) {
	const char *token_delimiters = " \t:;,(){}=-+\n";
	for (u32 i = 0; i < strlen(token_delimiters); i++) {
		if (character == token_delimiters[i]) {
			return true;
		}
	}
	return false;
}

typedef struct Stream {
	char *string;
	char *name;
	u32 line;
} Stream;

typedef struct Token {
	char *string;
	u32 length;
} Token;

#define Stream_Error(stream, format_string, ...) "Stream error: in %s on line %u: " format_string, (stream)->name, (stream)->line, ##__VA_ARGS__

bool Token_Equals(Token token, const char *comparand) {
	u32 i = 0;
	for (; (i < token.length) && comparand[i]; i++) {
		if (token.string[i] != comparand[i]) {
			return false;
		}
	}
	if (i != token.length || comparand[i] != '\0') {
		return false;
	}
	return true;
}

Token Do_Get_Token(Stream *stream) {
	while (*stream->string && isspace(*stream->string)) {
		if (*stream->string == '\n') {
			stream->line++;
		}
		stream->string++;
	}
	Token token = {
		.string = stream->string,
	};
	if (Is_Delimiter(*stream->string)) {
		stream->string++;
		token.length++;
		return token;
	}
	while (*stream->string && !Is_Delimiter(*stream->string)) {
		stream->string++;
		token.length++;
	}
	return token;
}

Token Get_Token(Stream *stream) {
	Token token = Do_Get_Token(stream);
	if (token.length == 0) {
		Abort("Unexpected end of stream");
	}
	return token;
}

bool Try_To_Get_Token(Stream *stream, Token *token) {
	*token = Do_Get_Token(stream);
	if (token->length == 0) {
		return false;
	}
	return true;
}

// @TODO: Get rid of this function?
void Get_Token_Into_Buffer(Stream *stream, char *buffer) {
	Token token = Get_Token(stream);
	for (u32 i = 0; i < token.length; i++) {
		buffer[i] = token.string[i];
	}
}

void Get_Expected_Token(Stream *stream, const char *expected) {
	Token token = Get_Token(stream);
	if (!Token_Equals(token, expected)) {
		Abort(Stream_Error(stream, "expected: '%s', got: '%.*s'", expected, token.length, token.string));
	}
}

void Get_Main_Block(Stream *stream, char *buffer, u32 buffer_length) {
	char *buffer_start = buffer;
	Get_Expected_Token(stream, "{");
	if (*stream->string == '\n') {
		stream->string++;
	}
	s32 unmatched_brace_count = 1;
	while (unmatched_brace_count > 0) {
		if (*stream->string == '{') {
			unmatched_brace_count++;
		}
		if (*stream->string == '}') {
			unmatched_brace_count--;
		}
		if (unmatched_brace_count > 0) {
			*buffer++ = *stream->string;
			assert(buffer - buffer_start < buffer_length);
		}
		stream->string++;
	}
}

char Peek_At_Next_Character(Stream *stream) {
	s32 i = 0;
	while (isspace(stream->string[i])) {
		i++;
	}
	return stream->string[i];
}

f32 Parse_Float_Value(Stream *stream) {
	Get_Expected_Token(stream, ":");
	char *new_stream_position;
	f32 result = strtof(stream->string, &new_stream_position);
	stream->string += (new_stream_position - stream->string);
	return result;
}

typedef struct Color {
	f32 r, g, b, a;
} Color;

Color Parse_Color_Value(Stream *stream) {
	Color color;
	Get_Expected_Token(stream, ":");
	Get_Expected_Token(stream, "{");
	Token token;
	while ((token = Get_Token(stream)), !Token_Equals(token, "}")) {
		if (Token_Equals(token, "r")) {
			color.r = Parse_Float_Value(stream);
		} else if (Token_Equals(token, "g")) {
			color.g = Parse_Float_Value(stream);
		} else if (Token_Equals(token, "b")) {
			color.b = Parse_Float_Value(stream);
		} else if (Token_Equals(token, "a")) {
			color.a = Parse_Float_Value(stream);
		} else {
			Abort("Unexpected token %s while parsing color", token);
		}
		Get_Expected_Token(stream, ",");
	}
	return color;
}

Token Parse_String_Value(Stream *stream) {
	Get_Expected_Token(stream, ":");
	return Get_Token(stream);
}

#define MAX_INPUTS_PER_BLOCK 32
#define MAX_OUTPUTS_PER_BLOCK 32
#define MAX_UNIFORMS_PER_SET 32
#define MAX_UBO_MEMBERS_PER_SET 32
#define MAX_SHADER_BLOCKS 1024
#define MAX_FILEPATH_LENGTH 256
#define MAX_MATERIALS 256
#define MAX_SHADERS 512
#define MAX_SHADER_BLOCK_MAIN_LENGTH 1024
#define MAX_BLOCKS_PER_SHADER 16

typedef enum Coordinate_System {
	INVALID_COORDINATE_SYSTEM,
	WORLD_COORDINATE_SYSTEM,
} Coordinate_System;

typedef struct Material {
	char name[256];
	bool has_color;
	Color color;
	Coordinate_System coordinate_system;
} Material;

typedef enum Shader_Stage {
	VERTEX_SHADER_STAGE,
	FRAGMENT_SHADER_STAGE,

	SHADER_STAGE_COUNT
} Shader_Stage;

typedef enum Uniform_Set {
	BIND_PER_MATERIAL_UPDATE_NEVER_UNIFORM_SET,
	BIND_PER_MATERIAL_UPDATE_DELAYABLE_UNIFORM_SET,
	BIND_PER_MATERIAL_UPDATE_IMMEDIATE_UNIFORM_SET,

	BIND_PER_OBJECT_UPDATE_NEVER_UNIFORM_SET,
	BIND_PER_OBJECT_UPDATE_DELAYABLE_UNIFORM_SET,
	BIND_PER_OBJECT_UPDATE_IMMEDIATE_UNIFORM_SET,

	UNIFORM_SET_COUNT
} Uniform_Set;

typedef enum Uniform_Update_Policy {
	UPDATE_UNIFORM_NEVER, // This uniform will be set at initialization and then never updated.
	UPDATE_UNIFORM_DELAYABLE, // This uniform will be updated, but updates do not need to be written immediately.
	UPDATE_UNIFORM_IMMEDIATE, // This uniform will be updated and updates should be written immediately.
} Uniform_Update_Policy;

typedef enum Descriptor_Type {
	UNIFORM_BUFFER_DESCRIPTOR,
	SAMPLER_DESCRIPTOR,
	COMBINED_IMAGE_SAMPLER_DESCRIPTOR,
	SAMPLED_IMAGE_DESCRIPTOR,

	DESCRIPTOR_TYPE_COUNT
} Descriptor_Type;

typedef struct Shader_Resource {
	char type[64];
	char name[64];
} Shader_Resource;

typedef struct Shader_Block_Stage {
	s32 input_count;
	Shader_Resource inputs[MAX_INPUTS_PER_BLOCK];

	s32 output_count;
	Shader_Resource outputs[MAX_OUTPUTS_PER_BLOCK];

	s32 uniform_counts[UNIFORM_SET_COUNT];
	Shader_Resource uniforms[UNIFORM_SET_COUNT][MAX_UNIFORMS_PER_SET];
	Descriptor_Type uniform_descriptor_types[UNIFORM_SET_COUNT][MAX_UNIFORMS_PER_SET];

	s32 ubo_member_counts[UNIFORM_SET_COUNT];
	Shader_Resource ubo_members[UNIFORM_SET_COUNT][MAX_UBO_MEMBERS_PER_SET];

	char main[MAX_SHADER_BLOCK_MAIN_LENGTH];
} Shader_Block_Stage;

typedef enum Shader_Block_ID {
	COLOR_SHADER_BLOCK,
	WORLD_SPACE_SHADER_BLOCK,

	SHADER_BLOCK_ID_COUNT,
} Shader_Block_ID;

typedef struct Shader_Block {
	char name[256];
	Shader_Block_ID id;
	bool stages_used[SHADER_STAGE_COUNT];
	Shader_Block_Stage stages[SHADER_STAGE_COUNT];
} Shader_Block;

const char *Get_File_Extension(const char *filename) {
	const char *result = NULL;
	while (*filename) {
		if (*filename == '.') {
			result = filename;
		}
		filename++;
	}
	return result;
}

bool Strings_Equal(const char *a, const char *b) {
	return strcmp(a, b) == 0;
}

const char *Shader_Stage_To_String(Shader_Stage stage) {
	switch (stage) {
	case VERTEX_SHADER_STAGE: {
		return "Vertex";
	} break;
	case FRAGMENT_SHADER_STAGE: {
		return "Fragment";
	} break;
	default: {
		Abort("invalid shader stage: %d", stage);
	} break;
	}
	return NULL;
}

const char *Uniform_Set_To_String(Uniform_Set set) {
	switch (set) {
	case BIND_PER_MATERIAL_UPDATE_NEVER_UNIFORM_SET: {
		return "Bind_Per_Material_Update_Never";
	} break;
	case BIND_PER_MATERIAL_UPDATE_DELAYABLE_UNIFORM_SET: {
		return "Bind_Per_Material_Update_Delayed";
	} break;
	case BIND_PER_MATERIAL_UPDATE_IMMEDIATE_UNIFORM_SET: {
		return "Bind_Per_Material_Update_Immediate";
	} break;
	case BIND_PER_OBJECT_UPDATE_NEVER_UNIFORM_SET: {
		return "Bind_Per_Object_Update_Never";
	} break;
	case BIND_PER_OBJECT_UPDATE_DELAYABLE_UNIFORM_SET: {
		return "Bind_Per_Object_Update_Delayable";
	} break;
	case BIND_PER_OBJECT_UPDATE_IMMEDIATE_UNIFORM_SET: {
		return "Bind_Per_Object_Update_Immediate";
	} break;
	default: {
		Abort("unknown uniform set");
	} break;
	}
	return NULL;
}

Uniform_Update_Policy Get_Uniform_Update_Policy_From_Set(Uniform_Set set) {
	switch (set) {
	case BIND_PER_MATERIAL_UPDATE_NEVER_UNIFORM_SET:
	case BIND_PER_OBJECT_UPDATE_NEVER_UNIFORM_SET: {
		return UPDATE_UNIFORM_NEVER;
	} break;
	case BIND_PER_MATERIAL_UPDATE_DELAYABLE_UNIFORM_SET:
	case BIND_PER_OBJECT_UPDATE_DELAYABLE_UNIFORM_SET: {
		return UPDATE_UNIFORM_DELAYABLE;
	} break;
	case BIND_PER_MATERIAL_UPDATE_IMMEDIATE_UNIFORM_SET:
	case BIND_PER_OBJECT_UPDATE_IMMEDIATE_UNIFORM_SET: {
		return UPDATE_UNIFORM_IMMEDIATE;
	} break;
	default: {
		Abort("unknown uniform set");
	} break;
	}
	return 0;
}

s32 Parse_Materials(Material *materials) {
	const char *materials_directory = "data/materials";
	s32 material_count = 0;
	Token token;
	Directory_Iteration materials_directory_iteration = {};
	while (Iterate_Through_All_Files_In_Directory(materials_directory, &materials_directory_iteration)) {
		materials[material_count] = (Material){};
		static char material_subdirectory[256];
		Join_Filepaths(materials_directory, materials_directory_iteration.filename, material_subdirectory);
		Directory_Iteration material_subdirectory_iteration = {};
		while (Iterate_Through_All_Files_In_Directory(material_subdirectory, &material_subdirectory_iteration)) {
			if (!Strings_Equal(Get_File_Extension(material_subdirectory_iteration.filename), ".mat")) {
				continue;
			}
			static char material_file_contents[512 * 512];
			Stream stream = {
				.string = material_file_contents,
				.line = 1,
				.name = materials_directory_iteration.filename,
			};
			static char material_file_path[256];
			Join_Filepaths(material_subdirectory, material_subdirectory_iteration.filename, material_file_path);
			Read_Entire_File(material_file_path, material_file_contents);
			strcpy(materials[material_count].name, materials_directory_iteration.filename);
			while (Try_To_Get_Token(&stream, &token)) {
				if (Token_Equals(token, "color")) {
					materials[material_count].has_color = true;
					materials[material_count].color = Parse_Color_Value(&stream);
				} else if (Token_Equals(token, "coordinate_system")) {
					Token coordinate_system = Parse_String_Value(&stream);
					if (Token_Equals(coordinate_system, "world")) {
						materials[material_count].coordinate_system = WORLD_COORDINATE_SYSTEM;
					} else {
						Abort("Unknown coordinate system '%s'", coordinate_system);
					}
				}
			}
			material_count++;
			assert(material_count < MAX_MATERIALS);
		}
	}
	return material_count;
}

#define Array_Count(x) (sizeof(x) / sizeof(x[0]))

s32 Parse_Shader_Blocks(Shader_Block *shader_blocks) {
	const char *shader_block_names[] = {
		[COLOR_SHADER_BLOCK] = "flat_color",
		[WORLD_SPACE_SHADER_BLOCK] = "world_space",
	};
	assert(Array_Count(shader_block_names) == SHADER_BLOCK_ID_COUNT);

	Token token;
	static char file_contents[512 * 512];
	Shader_Block_Stage *shader_block_stage = NULL;
	for (s32 i = 0; i < Array_Count(shader_block_names); i++) {
		static char shader_block_filepath[MAX_FILEPATH_LENGTH];
		sprintf(shader_block_filepath, "code/shader_blocks/%s.glsl", shader_block_names[i]);
		Read_Entire_File(shader_block_filepath, file_contents);
		Stream stream = {
			.string = file_contents,
			.line = 1,
			.name = shader_block_filepath,
		};

		strcpy(shader_blocks[i].name, shader_block_names[i]);

		shader_blocks[i].id = i;

		while (Try_To_Get_Token(&stream, &token)) {
			if (Token_Equals(token, "@VERTEX")) {
				shader_blocks[i].stages_used[VERTEX_SHADER_STAGE] = true;
				shader_block_stage = &shader_blocks[i].stages[VERTEX_SHADER_STAGE];
				continue;
			} else if (Token_Equals(token, "@FRAGMENT")) {
				shader_blocks[i].stages_used[FRAGMENT_SHADER_STAGE] = true;
				shader_block_stage = &shader_blocks[i].stages[FRAGMENT_SHADER_STAGE];
				continue;
			}
			assert(shader_block_stage);
			if (Token_Equals(token, "UNIFORMS")) {
				Get_Expected_Token(&stream, ":");
				Get_Expected_Token(&stream, "{");
				while (Peek_At_Next_Character(&stream) != '}') {
					Token type_token = Get_Token(&stream);
					Token name_token = Get_Token(&stream);
					Get_Expected_Token(&stream, ";");
					Token bind_token = Get_Token(&stream);
					Token update_token = Get_Token(&stream);
					Uniform_Set set;
					if (Token_Equals(bind_token, "@BIND_PER_MATERIAL") && Token_Equals(update_token, "@UPDATE_NEVER")) {
						set = BIND_PER_MATERIAL_UPDATE_NEVER_UNIFORM_SET;
					} else if (Token_Equals(bind_token, "@BIND_PER_MATERIAL") && Token_Equals(update_token, "@UPDATE_DELAYABLE")) {
						set = BIND_PER_MATERIAL_UPDATE_DELAYABLE_UNIFORM_SET;
					} else if (Token_Equals(bind_token, "@BIND_PER_MATERIAL") && Token_Equals(update_token, "@UPDATE_IMMEDIATE")) {
						set = BIND_PER_MATERIAL_UPDATE_IMMEDIATE_UNIFORM_SET;
					} else if (Token_Equals(bind_token, "@BIND_PER_OBJECT") && Token_Equals(update_token, "@UPDATE_NEVER")) {
						set = BIND_PER_OBJECT_UPDATE_NEVER_UNIFORM_SET;
					} else if (Token_Equals(bind_token, "@BIND_PER_OBJECT") && Token_Equals(update_token, "@UPDATE_DELAYABLE")) {
						set = BIND_PER_OBJECT_UPDATE_DELAYABLE_UNIFORM_SET;
					} else if (Token_Equals(bind_token, "@BIND_PER_OBJECT") && Token_Equals(update_token, "@UPDATE_IMMEDIATE")) {
						set = BIND_PER_OBJECT_UPDATE_IMMEDIATE_UNIFORM_SET;
					} else {
						Abort(Stream_Error(&stream, "invalid uniform set tokens, got %.*s and %.*s", bind_token.length, bind_token.string, update_token.length, update_token.string));
					}
					Shader_Resource *uniform;
					if (Token_Equals(type_token, "texture2D")) {
						uniform = &shader_block_stage->uniforms[set][shader_block_stage->uniform_counts[set]];
						shader_block_stage->uniform_descriptor_types[set][shader_block_stage->uniform_counts[set]] = SAMPLED_IMAGE_DESCRIPTOR;
						shader_block_stage->uniform_counts[set]++;
					} else {
						uniform = &shader_block_stage->ubo_members[set][shader_block_stage->ubo_member_counts[set]];
						shader_block_stage->ubo_member_counts[set]++;
					}
					strncpy(uniform->type, type_token.string, type_token.length);
					strncpy(uniform->name, name_token.string, name_token.length);
				}
				Get_Expected_Token(&stream, "}");
			} else if (Token_Equals(token, "INPUT")) {
				Get_Expected_Token(&stream, ":");
				Get_Expected_Token(&stream, "{");
				while (Peek_At_Next_Character(&stream) != '}') {
					Get_Token_Into_Buffer(&stream, shader_block_stage->inputs[shader_block_stage->input_count].type);
					Get_Token_Into_Buffer(&stream, shader_block_stage->inputs[shader_block_stage->input_count].name);
					Get_Expected_Token(&stream, ";");
					shader_block_stage->input_count++;
				}
				Get_Expected_Token(&stream, "}");
			} else if (Token_Equals(token, "OUTPUT")) {
				Get_Expected_Token(&stream, ":");
				Get_Expected_Token(&stream, "{");
				while (Peek_At_Next_Character(&stream) != '}') {
					Get_Token_Into_Buffer(&stream, shader_block_stage->outputs[shader_block_stage->output_count].type);
					Get_Token_Into_Buffer(&stream, shader_block_stage->outputs[shader_block_stage->output_count].name);
					Get_Expected_Token(&stream, ";");
					shader_block_stage->output_count++;
				}
				Get_Expected_Token(&stream, "}");
			} else if (Token_Equals(token, "MAIN")) {
				Get_Expected_Token(&stream, ":");
				Get_Main_Block(&stream, shader_block_stage->main, sizeof(shader_block_stage->main));
			} else {
				Abort(Stream_Error(&stream, "Unknown shader block field: %.*s", token.length, token.string));
			}
		}
	}
	return Array_Count(shader_block_names);
}

const char *SHADER_CODE_DIRECTORY = "build/shaders/code";
const char *SHADER_BINARY_DIRECTORY = "build/shaders/binaries";

typedef struct Shader_Block_Group {
	bool stages_used[SHADER_STAGE_COUNT];
	s32 ubo_member_counts_per_set[SHADER_STAGE_COUNT][UNIFORM_SET_COUNT];
	bool uniform_sets_used[SHADER_STAGE_COUNT][UNIFORM_SET_COUNT];

	s32 block_count;
	Shader_Block *blocks[MAX_BLOCKS_PER_SHADER];
} Shader_Block_Group;

void Output_Shader(const char *shader_name, Shader_Block_Group *block_group) {
	static char shader_filepath[MAX_FILEPATH_LENGTH];
	sprintf(shader_filepath, "%s/%s.glsl", SHADER_CODE_DIRECTORY, shader_name);
	FILE *shader_file = fopen(shader_filepath, "w");
	fprintf(shader_file, "// This file was auto-generated by material_compiler.c.\n\n");
	fprintf(shader_file, "// Shader blocks:\n");
	for (s32 i = 0; i < block_group->block_count; i++) {
		fprintf(shader_file, "//    %s\n", block_group->blocks[i]->name);
	}
	fprintf(shader_file, "\n");

	fprintf(shader_file, "#version 420\n\n");
	for (s32 i = 0; i < SHADER_STAGE_COUNT; i++) {
		// @TODO: Store a list of used stages rather than a bool array.
		if (!block_group->stages_used[i]) {
			continue;
		}

		switch (i) {
		case VERTEX_SHADER_STAGE: {
			fprintf(shader_file, "#if defined(VERTEX_SHADER)\n\n");
		} break;
		case FRAGMENT_SHADER_STAGE: {
			fprintf(shader_file, "#if defined(FRAGMENT_SHADER)\n\n");
		} break;
		default: {
			Abort("unknown shader stage");
		} break;
		}

		s32 total_input_count = 0;
		for (s32 j = 0; j < block_group->block_count; j++) {
			for (s32 k = 0; k < block_group->blocks[j]->stages[i].input_count; k++) {
				fprintf(shader_file, "layout (location = %d) in %s %s; // %s\n", total_input_count, block_group->blocks[j]->stages[i].inputs[k].type, block_group->blocks[j]->stages[i].inputs[k].name, block_group->blocks[j]->name);
				total_input_count++;
			}
		}
		if (total_input_count > 0) {
			fprintf(shader_file, "\n");
		}

		s32 total_output_count = 0;
		for (s32 j = 0; j < block_group->block_count; j++) {
			for (s32 k = 0; k < block_group->blocks[j]->stages[i].output_count; k++) {
				fprintf(shader_file, "layout (location = %d) out %s %s; // %s\n", total_output_count, block_group->blocks[j]->stages[i].outputs[k].type, block_group->blocks[j]->stages[i].outputs[k].name, block_group->blocks[j]->name);
				total_output_count++;
			}
		}
		if (total_output_count > 0) {
			fprintf(shader_file, "\n");
		}

		s32 set_number = 0;
		for (s32 j = 0; j < UNIFORM_SET_COUNT; j++) {
			s32 binding_number = 0;
			if (block_group->ubo_member_counts_per_set[i][j] > 0) {
				fprintf(shader_file, "layout (set = %d, binding = %d) uniform %s_UBO {\n", set_number, binding_number, Uniform_Set_To_String(j));
				for (s32 k = 0; k < block_group->block_count; k++) {
					for (s32 l = 0; l < block_group->blocks[k]->stages[i].ubo_member_counts[j]; l++) {
						fprintf(shader_file, "\t");
						if (Strings_Equal(block_group->blocks[k]->stages[i].ubo_members[j][l].type, "mat4")) {
							fprintf(shader_file, "layout(row_major) ");
						}
						fprintf(shader_file, "%s %s; // %s\n", block_group->blocks[k]->stages[i].ubo_members[j][l].type, block_group->blocks[k]->stages[i].ubo_members[j][l].name, block_group->blocks[k]->name);
					}
				}
				fprintf(shader_file, "};\n");
				binding_number++;
			}
			for (s32 k = 0; k < block_group->block_count; k++) {
				for (s32 l = 0; l < block_group->blocks[k]->stages[i].uniform_counts[j]; l++) {
					fprintf(shader_file, "layout (set = %d, binding = %d", set_number, binding_number);
					if (Strings_Equal(block_group->blocks[k]->stages[i].uniforms[j][l].type, "mat4")) {
						fprintf(shader_file, ", row_major");
					}
					fprintf(shader_file, ") uniform %s %s; // %s\n", block_group->blocks[k]->stages[i].uniforms[j][l].type, block_group->blocks[k]->stages[i].uniforms[j][l].name, block_group->blocks[k]->name);
					binding_number++;
				}
			}
			if (binding_number > 0) {
				set_number++;
				fprintf(shader_file, "\n");
			}
		}

		fprintf(shader_file, "void main() {\n");
		for (s32 j = 0; j < block_group->block_count; j++) {
			if (block_group->blocks[j]->stages[i].main[0] == '\0') {
				continue;
			}
			if (j != 0) {
				fprintf(shader_file, "\n");
			}
			fprintf(shader_file, "	// %s\n"
			                     "	{\n"
			                     "	%s"
			                     "	}\n", block_group->blocks[j]->name, block_group->blocks[j]->stages[i].main);
		}
		fprintf(shader_file, "}\n\n");
		fprintf(shader_file, "#endif\n\n\n");
	}
	fclose(shader_file);
}

void Add_Shader_Block_To_Group(Shader_Block *block, Shader_Block_Group *block_group) {
	for (s32 i = 0; i < SHADER_STAGE_COUNT; i++) {
		if (block->stages_used[i]) {
			block_group->stages_used[i] = true;
		}
	}
	for (s32 i = 0; i < SHADER_STAGE_COUNT; i++) {
		if (!block->stages_used[i]) {
			continue;
		}
		for (s32 j = 0; j < UNIFORM_SET_COUNT; j++) {
			block_group->ubo_member_counts_per_set[i][j] += block->stages[i].ubo_member_counts[j];
			if (block->stages[i].uniform_counts[j] > 0 || block->stages[i].ubo_member_counts[j] > 0) {
				block_group->uniform_sets_used[i][j] = true;
			}
		}
	}
	block_group->blocks[block_group->block_count] = block;
	block_group->block_count++;
}

s32 Generate_Shader_Code(s32 material_count, Material *materials, char *shader_names[MAX_SHADERS], Shader_Block_Group block_groups[MAX_SHADERS]) {
	static Shader_Block shader_blocks[MAX_SHADER_BLOCKS];
	s32 shader_block_count = Parse_Shader_Blocks(shader_blocks);
	s32 shader_count = 0;
	for (s32 i = 0; i < material_count; i++) {
		if (materials[i].has_color) {
			Add_Shader_Block_To_Group(&shader_blocks[COLOR_SHADER_BLOCK], &block_groups[shader_count]);
		}
		switch (materials[i].coordinate_system) {
		case WORLD_COORDINATE_SYSTEM: {
			Add_Shader_Block_To_Group(&shader_blocks[WORLD_SPACE_SHADER_BLOCK], &block_groups[shader_count]);
		} break;
		case INVALID_COORDINATE_SYSTEM: {
			Abort("No coordinate system assigned to material %s", materials[i].name);
		} break;
		}
		assert(block_groups[i].block_count < MAX_BLOCKS_PER_SHADER);
		shader_names[i] = materials[i].name;
		Output_Shader(shader_names[shader_count], &block_groups[shader_count]);
		shader_count++;
	}
	assert(shader_count < MAX_SHADERS);
	return shader_count;
}

void Compile_Shader_Code(s32 shader_count, char *shader_names[MAX_SHADERS], Shader_Block_Group block_groups[MAX_SHADERS]) {
	for (s32 i = 0; i < shader_count; i++) {
		static char command[256];
		for (s32 j = 0; j < SHADER_STAGE_COUNT; j++) {
			if (!block_groups[i].stages_used[j]) {
				continue;
			}
			const char *stage_define, *stage;
			switch (j) {
			case VERTEX_SHADER_STAGE: {
				stage_define = "VERTEX_SHADER";
				stage = "vert";
			} break;
			case FRAGMENT_SHADER_STAGE: {
				stage_define = "FRAGMENT_SHADER";
				stage = "frag";
			} break;
			default: {
				Abort("Unknown shader stage");
			} break;
			}
			sprintf(command, "$VULKAN_SDK_PATH/bin/glslangValidator -V %s/%s.glsl -D%s -S %s -o %s/%s_%s.spirv", SHADER_CODE_DIRECTORY, shader_names[i], stage_define, stage, SHADER_BINARY_DIRECTORY, shader_names[i], stage);
			if (system(command) != 0) {
				Abort("Shader compilation command failed: %s", command);
			}
		}
	}
}

char *Uppercase_String(const char *source, char *destination) {
	char *result = destination;
	while (*source) {
		*destination++ = toupper(*source++);
	}
	*destination = '\0';
	return result;
}

char *Lowercase_String(const char *source, char *destination) {
	char *result = destination;
	while (*source) {
		*destination++ = tolower(*source++);
	}
	*destination = '\0';
	return result;
}

char *Get_Descriptor_Set_Name(const char *shader_name, Shader_Stage stage, Uniform_Set set, char *destination) {
	s32 i = sprintf(destination, "%s_", shader_name);
	i += sprintf(destination + i, "%s_", Shader_Stage_To_String(stage));
	i += sprintf(destination + i, "%s", Uniform_Set_To_String(set));
	Lowercase_String(destination, destination);
	return destination;
}

#define MAX_UNIFORMS_PER_SHADER 32
#define MAX_DESCRIPTORS_PER_SET 16
#define MAX_DESCRIPTOR_SETS 16

void Generate_Render_Code(s32 shader_count, char *shader_names[MAX_SHADERS], Shader_Block_Group block_groups[MAX_SHADERS]) {
	char buffer[128];

	s32 descriptor_set_layout_count = 0;
	struct {
		Uniform_Set uniform_set;
		Shader_Stage stage;
		s32 descriptor_count;
		Descriptor_Type descriptor_types[MAX_DESCRIPTORS_PER_SET];
	} descriptor_set_layouts[MAX_SHADERS][MAX_DESCRIPTOR_SETS];
	struct {
		s32 count;
		Shader_Resource *resources[MAX_UNIFORMS_PER_SHADER];
	} uniforms[MAX_SHADERS];
	s32 immediate_set_count = 0;
	s32 non_immediate_set_count = 0;
	s32 immediate_descriptor_counts_by_type[DESCRIPTOR_TYPE_COUNT] = {};
	s32 non_immediate_descriptor_counts_by_type[DESCRIPTOR_TYPE_COUNT] = {};
	for (s32 i = 0; i < shader_count; i++) {
		for (s32 j = 0; j < SHADER_STAGE_COUNT; j++) {
			for (s32 k = 0; k < UNIFORM_SET_COUNT; k++) {
				if (!block_groups[i].uniform_sets_used[j][k]) {
					continue;
				}
				descriptor_set_layouts[i][descriptor_set_layout_count].stage = j;
				descriptor_set_layouts[i][descriptor_set_layout_count].uniform_set = k;
				for (s32 l = 0; l < block_groups[i].block_count; l++) {
					for (s32 m = 0; m < block_groups[i].blocks[l]->stages[j].ubo_member_counts[k]; m++) {
						uniforms[i].resources[uniforms[i].count] = &block_groups[i].blocks[l]->stages[j].ubo_members[k][m];
						uniforms[i].count++;
					}
				}
				if (block_groups[i].ubo_member_counts_per_set[j][k] > 0) {
					descriptor_set_layouts[i][descriptor_set_layout_count].descriptor_types[descriptor_set_layouts[i][descriptor_set_layout_count].descriptor_count] = UNIFORM_BUFFER_DESCRIPTOR;
					descriptor_set_layouts[i][descriptor_set_layout_count].descriptor_count++;
				}
				for (s32 l = 0; l < block_groups[i].block_count; l++) {
					for (s32 m = 0; m < block_groups[i].blocks[l]->stages[j].uniform_counts[k]; m++) {
						Descriptor_Type type = block_groups[i].blocks[l]->stages[j].uniform_descriptor_types[k][m];
						descriptor_set_layouts[i][descriptor_set_layout_count].descriptor_types[descriptor_set_layouts[i][descriptor_set_layout_count].descriptor_count] = type;
						descriptor_set_layouts[i][descriptor_set_layout_count].descriptor_count++;

						uniforms[i].resources[uniforms[i].count] = &block_groups[i].blocks[l]->stages[j].uniforms[k][m];
						uniforms[i].count++;
					}
				}
				for (s32 l = 0; l < descriptor_set_layouts[i][descriptor_set_layout_count].descriptor_count; l++) {
					Descriptor_Type type = descriptor_set_layouts[i][descriptor_set_layout_count].descriptor_types[l];
					if (Get_Uniform_Update_Policy_From_Set(k) == UPDATE_UNIFORM_IMMEDIATE) {
						immediate_descriptor_counts_by_type[type]++;
						immediate_set_count++;
					} else {
						non_immediate_descriptor_counts_by_type[type]++;
						non_immediate_set_count++;
					}
				}
				assert(descriptor_set_layouts[i][descriptor_set_layout_count].descriptor_count < MAX_DESCRIPTORS_PER_SET);
				descriptor_set_layout_count++;
				assert(descriptor_set_layout_count < MAX_DESCRIPTOR_SETS);
			}
		}
	}

	// Vulkan header file.
	{
		FILE *vulkan_header_file = fopen("code/vulkan_generated.h", "w");
		assert(vulkan_header_file);

		fprintf(vulkan_header_file, "// This file was auto-generated by material_compiler.c.\n\n");

		fprintf(vulkan_header_file, "typedef enum GPU_Shader_Stage_Flags {\n");
		for (s32 i = 0; i < SHADER_STAGE_COUNT; i++) {
			const char *member;
			switch (i) {
			case VERTEX_SHADER_STAGE: {
				member = "GPU_VERTEX_SHADER_STAGE = VK_SHADER_STAGE_VERTEX_BIT";
			} break;
			case FRAGMENT_SHADER_STAGE: {
				member = "GPU_FRAGMENT_SHADER_STAGE = VK_SHADER_STAGE_FRAGMENT_BIT";
			} break;
			default: {
				Abort("unknown shader stage: %d", i);
			} break;
			}
			fprintf(vulkan_header_file, "	%s,\n", member);
		}
		fprintf(vulkan_header_file, "\n	GPU_SHADER_STAGE_COUNT\n");
		fprintf(vulkan_header_file, "} GPU_Shader_Stage_Flags;\n\n");

		fprintf(vulkan_header_file, "typedef enum GPU_Shader_ID {\n");
		for (s32 i = 0; i < shader_count; i++) {
			fprintf(vulkan_header_file, "	%s_SHADER,\n", Uppercase_String(shader_names[i], buffer));
		}
		fprintf(vulkan_header_file, "\n	GPU_SHADER_COUNT,\n");
		fprintf(vulkan_header_file, "} GPU_Shader_ID;\n\n");

		//fprintf(vulkan_header_file, "typedef enum GPU_Shader_Stage {\n"
							 //"	VERTEX_SHADER_STAGE,\n"
							 //"	FRAGMENT_SHADER_STAGE,\n"
							 //"	SHADER_STAGE_COUNT,\n"
							 //"} GPU_Shader_Stage;\n\n");

		fprintf(vulkan_header_file, "typedef struct GPU_Shader {\n"
							 "	s32 stage_count;\n"
							 "	GPU_Shader_Stage_Flags stages[GPU_SHADER_STAGE_COUNT];\n"
							 "	VkShaderModule modules[GPU_SHADER_STAGE_COUNT];\n"
							 "} GPU_Shader;\n\n");

		fprintf(vulkan_header_file, "typedef enum GPU_Descriptor_Set_ID {\n");
		for (s32 i = 0; i < shader_count; i++) {
			for (s32 j = 0; j < descriptor_set_layout_count; j++) {
				fprintf(vulkan_header_file, "	%s_DESCRIPTOR_SET,\n", Uppercase_String(Get_Descriptor_Set_Name(shader_names[i], descriptor_set_layouts[i][j].stage, descriptor_set_layouts[i][j].uniform_set, buffer), buffer));
			}
		}
		fprintf(vulkan_header_file, "} GPU_Descriptor_Set_ID;\n\n");

		for (s32 i = 0; i < shader_count; i++) {
			fprintf(vulkan_header_file, "#define %s_DESCRIPTOR_SET_LAYOUT_COUNT %d\n", Uppercase_String(shader_names[i], buffer), descriptor_set_layout_count);
		}
		fprintf(vulkan_header_file, "\n");

		fprintf(vulkan_header_file, "typedef struct Vulkan_Descriptor_Set_Layouts {\n");
		for (s32 i = 0; i < shader_count; i++) {
			fprintf(vulkan_header_file, "	VkDescriptorSetLayout %s[%s_DESCRIPTOR_SET_LAYOUT_COUNT];\n", shader_names[i], Uppercase_String(shader_names[i], buffer));
		}
		fprintf(vulkan_header_file, "} Vulkan_Descriptor_Set_Layouts;\n\n");

		fprintf(vulkan_header_file, "typedef struct GPU_Descriptor_Sets {\n"
		                     "	Vulkan_Descriptor_Set_Layouts layouts;\n");
		for (s32 i = 0; i < shader_count; i++) {
			for (s32 j = 0; j < descriptor_set_layout_count; j++) {
				if (Get_Uniform_Update_Policy_From_Set(descriptor_set_layouts[i][j].uniform_set) == UPDATE_UNIFORM_IMMEDIATE) {
					fprintf(vulkan_header_file, "	VkDescriptorSet *");
				} else {
					fprintf(vulkan_header_file, "	VkDescriptorSet ");
				}
				fprintf(vulkan_header_file, "%s;\n", Get_Descriptor_Set_Name(shader_names[i], descriptor_set_layouts[i][j].stage, descriptor_set_layouts[i][j].uniform_set, buffer));
			}
		}
		fprintf(vulkan_header_file, "} GPU_Descriptor_Sets;\n\n");

		fprintf(vulkan_header_file, "#define GPU_NON_IMMEDIATE_DESCRIPTOR_COUNT %d\n", non_immediate_set_count);
		fprintf(vulkan_header_file, "#define GPU_IMMEDIATE_DESCRIPTOR_COUNT %d\n", immediate_set_count);

		fclose(vulkan_header_file);
	}

	// Vulkan source file.
	{
		FILE *vulkan_source_file = fopen("code/vulkan_generated.c", "w");
		assert(vulkan_source_file);

		fprintf(vulkan_source_file, "// This file was auto-generated by material_compiler.c.\n\n");

		fprintf(vulkan_source_file, "const char *SHADER_SPIRV_FILEPATHS[GPU_SHADER_COUNT][GPU_SHADER_STAGE_COUNT] = {\n");
		for (s32 i = 0; i < shader_count; i++) {
			for (s32 j = 0; j < SHADER_STAGE_COUNT; j++) {
				if (!block_groups[i].stages_used[j]) {
					continue;
				}
				const char *stage_enum, *stage_name;
				switch (j) {
				case VERTEX_SHADER_STAGE: {
					stage_enum = "GPU_VERTEX_SHADER_STAGE";
					stage_name = "vert";
				} break;
				case FRAGMENT_SHADER_STAGE: {
					stage_enum = "GPU_FRAGMENT_SHADER_STAGE";
					stage_name = "frag";
				} break;
				default: {
					Abort("Unknown shader stage");
				} break;
				}
				fprintf(vulkan_source_file, "	[%s_SHADER][%s] = \"%s/%s_%s.spirv\",\n", Uppercase_String(shader_names[i], buffer), stage_enum, SHADER_BINARY_DIRECTORY, shader_names[i], stage_name);
			}
		}
		fprintf(vulkan_source_file, "};\n\n");

		fprintf(vulkan_source_file, "struct {\n"
		                     "	VkDescriptorType type;\n"
		                     "	s32 non_immediate_descriptor_count;\n"
		                     "	s32 immediate_descriptor_count;\n"
		                     "} vulkan_descriptor_pool_size_infos[] = {\n");
		for (s32 i = 0; i < DESCRIPTOR_TYPE_COUNT; i++) {
			if (non_immediate_descriptor_counts_by_type[i] == 0 && immediate_descriptor_counts_by_type[i] == 0) {
				continue;
			}
			const char *descriptor_type_string;
			switch (i) {
			case UNIFORM_BUFFER_DESCRIPTOR: {
				descriptor_type_string = "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER";
			} break;
			case COMBINED_IMAGE_SAMPLER_DESCRIPTOR: {
				descriptor_type_string = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER";
			} break;
			default: {
				Abort("invalid descriptor type: %d", i);
			} break;
			}
			fprintf(vulkan_source_file, "	{\n"
			                     "		.type = %s,\n"
			                     "		.immediate_descriptor_count = %d,\n"
			                     "		.non_immediate_descriptor_count = %d,\n"
			                     "	},\n",
			                     descriptor_type_string, non_immediate_descriptor_counts_by_type[i], immediate_descriptor_counts_by_type[i]);
		}
		fprintf(vulkan_source_file, "};\n\n");

		fprintf(vulkan_source_file, "void Render_API_Load_Shaders(Render_API_Context *context, GPU_Shader shaders[GPU_SHADER_COUNT]) {\n"
							 "	for (s32 i = 0; i < GPU_SHADER_COUNT; i++) {\n"
							 "		for (s32 j = 0; j < GPU_SHADER_STAGE_COUNT; j++) {\n"
							 "			if (!SHADER_SPIRV_FILEPATHS[i][j]) {\n"
							 "				continue;\n"
							 "			}\n"
							 "			Read_File_Result spirv = Read_Entire_File(SHADER_SPIRV_FILEPATHS[i][j]);\n"
							 "			Assert(spirv.data);\n"
							 "			VkShaderModuleCreateInfo shader_module_create_info = {\n"
							 "				.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,\n"
							 "				.codeSize = spirv.size,\n"
							 "				.pCode = (u32 *)spirv.data,\n"
							 "			};\n"
							 "			shaders[i].stages[shaders[i].stage_count] = j;\n"
							 "			VK_CHECK(vkCreateShaderModule(context->device, &shader_module_create_info, NULL, &shaders[i].modules[shaders[i].stage_count]));\n"
							 "			shaders[i].stage_count++;\n"
							 "		}\n"
							 "	}\n"
							 "}\n\n");

		fprintf(vulkan_source_file, "void Vulkan_Create_Descriptor_Sets(Render_API_Context *context, u32 swapchain_image_count, VkDescriptorPool descriptor_pool, GPU_Descriptor_Sets *descriptor_sets) {\n");
		fprintf(vulkan_source_file, "	// Create the descriptor set layouts.\n");
		for (s32 i = 0; i < shader_count; i++) {
			for (s32 j = 0; j < descriptor_set_layout_count; j++) {
				char *stage_flags;
				switch (descriptor_set_layouts[i][j].stage) {
				case VERTEX_SHADER_STAGE: {
					stage_flags = "VK_SHADER_STAGE_VERTEX_BIT";
				} break;
				case FRAGMENT_SHADER_STAGE: {
					stage_flags = "VK_SHADER_STAGE_FRAGMENT_BIT";
				} break;
				default: {
					Abort("unknown shader stage");
				} break;
				}
				s32 binding_number = 0;
				fprintf(vulkan_source_file, "	{\n");
				fprintf(vulkan_source_file, "		VkDescriptorSetLayoutBinding bindings[] = {\n");
				char *descriptor_type_string;
				for (s32 k = 0; k < descriptor_set_layouts[i][j].descriptor_count; k++) {
					switch (descriptor_set_layouts[i][j].descriptor_types[k]) {
					case UNIFORM_BUFFER_DESCRIPTOR: {
						descriptor_type_string = "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER";
					} break;
					case SAMPLER_DESCRIPTOR: {
						descriptor_type_string = "VK_DESCRIPTOR_TYPE_SAMPLER";
					} break;
					case COMBINED_IMAGE_SAMPLER_DESCRIPTOR: {
						descriptor_type_string = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER";
					} break;
					case SAMPLED_IMAGE_DESCRIPTOR: {
						descriptor_type_string = "VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE";
					} break;
					default: {
						Abort("invalid uniform descriptor type");
					} break;
					}
					fprintf(vulkan_source_file, "			{\n"
										 "				.binding = %d,\n"
										 "				.descriptorType = %s,\n"
										 "				.descriptorCount = 1,\n"
										 "				.stageFlags = %s,\n"
										 "				.pImmutableSamplers = NULL,\n"
										 "			},\n",
										 binding_number, descriptor_type_string, stage_flags);
					binding_number++;
				}
				fprintf(vulkan_source_file, "		};\n");
				fprintf(vulkan_source_file, "		VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {\n"
									 "			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,\n"
									 "			.bindingCount = Array_Count(bindings),\n"
									 "			.pBindings = bindings,\n"
									 "			.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,\n"
									 "		};\n");
				fprintf(vulkan_source_file, "		VK_CHECK(vkCreateDescriptorSetLayout(context->device, &descriptor_set_layout_create_info, NULL, &descriptor_sets->layouts.%s[%s_DESCRIPTOR_SET]));\n", shader_names[i], Uppercase_String(Get_Descriptor_Set_Name(shader_names[i], descriptor_set_layouts[i][j].stage, descriptor_set_layouts[i][j].uniform_set, buffer), buffer));
				fprintf(vulkan_source_file, "\t}\n");
			}
		}

		fprintf(vulkan_source_file, "\n	// Create the descriptor sets.\n");
		fprintf(vulkan_source_file, "	s32 layout_index = 0;\n");
		fprintf(vulkan_source_file, "	VkDescriptorSetLayout layouts[context->descriptor_set_count];\n");
		for (s32 i = 0; i < shader_count; i++) {
			for (s32 j = 0; j < descriptor_set_layout_count; j++) {
				if (Get_Uniform_Update_Policy_From_Set(descriptor_set_layouts[i][j].uniform_set) == UPDATE_UNIFORM_IMMEDIATE) {
					fprintf(vulkan_source_file, "	for (s32 i = 0; i != swapchain_image_count; i++) {\n"
 					                     "		layouts[layout_index++] = descriptor_sets->layouts.%s[%s_DESCRIPTOR_SET];\n"
 					                     "	}\n",
 					                     shader_names[i], Uppercase_String(Get_Descriptor_Set_Name(shader_names[i], descriptor_set_layouts[i][j].stage, descriptor_set_layouts[i][j].uniform_set, buffer), buffer));
				} else {
					fprintf(vulkan_source_file, "	layouts[layout_index++] = descriptor_sets->layouts.%s[%s_DESCRIPTOR_SET];\n", shader_names[i], Uppercase_String(Get_Descriptor_Set_Name(shader_names[i], descriptor_set_layouts[i][j].stage, descriptor_set_layouts[i][j].uniform_set, buffer), buffer));
				}
			}
		}
		fprintf(vulkan_source_file, "	VkDescriptorSet results[context->descriptor_set_count];\n");
		fprintf(vulkan_source_file, "	VkDescriptorSetAllocateInfo allocate_info = {\n"
		                     "		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,\n"
		                     "		.descriptorPool = descriptor_pool,\n"
		                     "		.descriptorSetCount = context->descriptor_set_count,\n"
		                     "		.pSetLayouts = layouts,\n"
		                     "	};\n");
		fprintf(vulkan_source_file, "	VK_CHECK(vkAllocateDescriptorSets(context->device, &allocate_info, results));\n");
		fprintf(vulkan_source_file, "	s32 result_index = 0;\n");
		for (s32 i = 0; i < shader_count; i++) {
			for (s32 j = 0; j < descriptor_set_layout_count; j++) {
				if (Get_Uniform_Update_Policy_From_Set(descriptor_set_layouts[i][j].uniform_set) == UPDATE_UNIFORM_IMMEDIATE) {
					fprintf(vulkan_source_file, "	descriptor_sets->%s = malloc(swapchain_image_count * sizeof(VkDescriptorSet)); // @TODO\n", Get_Descriptor_Set_Name(shader_names[i], descriptor_set_layouts[i][j].stage, descriptor_set_layouts[i][j].uniform_set, buffer));
					fprintf(vulkan_source_file, "	for (s32 i = 0; i < swapchain_image_count; i++) {\n");
					fprintf(vulkan_source_file, "		descriptor_sets->%s[i] = results[result_index++];\n", Get_Descriptor_Set_Name(shader_names[i], descriptor_set_layouts[i][j].stage, descriptor_set_layouts[i][j].uniform_set, buffer));
					fprintf(vulkan_source_file, "	}\n");
				} else {
					fprintf(vulkan_source_file, "	descriptor_sets->%s = results[result_index++];\n", Get_Descriptor_Set_Name(shader_names[i], descriptor_set_layouts[i][j].stage, descriptor_set_layouts[i][j].uniform_set, buffer));
				}
			}
		}

		fprintf(vulkan_source_file, "}\n\n");

		fclose(vulkan_source_file);
	}

	// Render source file.
	{
		FILE *render_source_file = fopen("code/render_generated.c", "w");
		assert(render_source_file);

		fprintf(render_source_file, "// This file was auto-generated by material_compiler.c.\n\n");

		fprintf(render_source_file, "void Create_Material_Pipelines(Render_API_Context *context, GPU_Descriptor_Sets *descriptor_sets, GPU_Shader shaders[GPU_SHADER_COUNT], GPU_Render_Pass scene_render_pass, GPU_Pipeline *pipelines) {\n");
		for (s32 i = 0; i < shader_count; i++) {
			// @TODO: Hardcode less of this stuff.
			fprintf(render_source_file, "	{\n"
			                     "		GPU_Pipeline_Description pipeline_description = {\n"
			                     "			.descriptor_set_layout_count = %s_DESCRIPTOR_SET_LAYOUT_COUNT,\n"
			                     "			.descriptor_set_layouts = descriptor_sets->layouts.%s,\n"
			                     "			.push_constant_count = 0,\n"
			                     "			.push_constant_descriptions = NULL,\n"
			                     "			.topology = GPU_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,\n"
			                     "			.viewport_width = window_width,\n"
			                     "			.viewport_height = window_width,\n"
			                     "			.scissor_width = window_width,\n"
			                     "			.scissor_height = window_width,\n"
			                     "			.depth_compare_operation = VK_COMPARE_OP_LESS,\n"
			                     "			.framebuffer_attachment_color_blend_count = 1,\n"
			                     "			.framebuffer_attachment_color_blend_descriptions = &(GPU_Framebuffer_Attachment_Color_Blend_Description){\n"
			                     "				.color_write_mask = GPU_COLOR_COMPONENT_RED | GPU_COLOR_COMPONENT_GREEN | GPU_COLOR_COMPONENT_BLUE | GPU_COLOR_COMPONENT_ALPHA,\n"
			                     "				.enable_blend = true,\n"
			                     "				.source_color_blend_factor = GPU_BLEND_FACTOR_SRC_ALPHA,\n"
			                     "				.destination_color_blend_factor = GPU_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,\n"
			                     "				//.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,\n"
			                     "				//.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,\n"
			                     "				.color_blend_operation = GPU_BLEND_OP_ADD ,\n"
			                     "				.source_alpha_blend_factor = GPU_BLEND_FACTOR_ONE,\n"
			                     "				.destination_alpha_blend_factor = GPU_BLEND_FACTOR_ZERO,\n"
			                     "				.alpha_blend_operation = VK_BLEND_OP_ADD,\n"
			                     "			},\n"
			                     "			.vertex_input_attribute_count = 1,\n"
			                     "			.vertex_input_attribute_descriptions = (GPU_Pipeline_Vertex_Input_Attribute_Description[]){\n"
			                     "				{\n"
			                     "					.binding = GPU_VERTEX_BUFFER_BIND_ID,\n"
			                     "					.location = 0,\n"
			                     "					.format = GPU_FORMAT_R32G32B32_SFLOAT,\n"
			                     "					.offset = 0,\n"
			                     "				},\n"
			                     "			},\n"
			                     "			.vertex_input_binding_count = 1,\n"
			                     "			.vertex_input_binding_descriptions = (GPU_Pipeline_Vertex_Input_Binding_Description[]){\n"
			                     "				{\n"
			                     "					.binding = GPU_VERTEX_BUFFER_BIND_ID,\n"
			                     "					.stride = sizeof(V3),\n"
			                     "					.input_rate = GPU_VERTEX_INPUT_RATE_VERTEX,\n"
			                     "				},\n"
			                     "			},\n"
			                     "			.dynamic_state_count = 2,\n"
			                     "			.dynamic_states = (GPU_Dynamic_Pipeline_State[]){\n"
			                     "				GPU_DYNAMIC_PIPELINE_STATE_VIEWPORT,\n"
			                     "				GPU_DYNAMIC_PIPELINE_STATE_SCISSOR,\n"
			                     "			},\n"
			                     "			.shader = shaders[%s_SHADER],\n"
			                     "			.render_pass = scene_render_pass,\n"
			                     "			.enable_depth_bias = false,\n"
			                     "		};\n"
			                     "		pipelines[%s_SHADER] = Render_API_Create_Pipeline(context, &pipeline_description);\n"
			                     "	}\n",
			                     Uppercase_String(shader_names[i], buffer), shader_names[i], buffer, buffer);
			fprintf(render_source_file, "}\n\n");
		}

		fprintf(render_source_file, "GPU_Shader_ID Get_Shader_ID_From_Material_ID() {\n");
		fprintf(render_source_file, "}\n\n");

		fprintf(render_source_file, "void Update_Descriptor_Sets(Render_Context *context, s32 descriptor_set_data_count, Descriptor_Set_Data *descriptor_set_data, Camera *camera) {\n"
		                            "	for (s32 i = 0; i < descriptor_set_data_count; i++) {\n"
		                            "	switch (descriptor_set_data[i].shader_id) {\n"
		                            "	}\n"
		                            "	}\n");
		fprintf(render_source_file, "}\n\n");

		fclose(render_source_file);
	}

	// Render header file.
	{
		FILE *render_header_file = fopen("code/render_generated.h", "w");
		assert(render_header_file);

		fprintf(render_header_file, "// This file was auto-generated by material_compiler.c.\n\n");

		fprintf(render_header_file, "typedef union Descriptor_Set_Parameters  {\n");
		for (s32 i = 0; i < shader_count; i++) {
			fprintf(render_header_file, "	struct {\n");
			for (s32 j = 0; j < block_groups[i].block_count; j++) {
				for (s32 k = 0; k < SHADER_STAGE_COUNT; k++) {
					for (s32 l = 0; l < UNIFORM_SET_COUNT; l++) {
						for (s32 m = 0; m < block_groups[i].blocks[j]->stages[k].uniform_counts[l]; m++) {
							if (Strings_Equal(block_groups[i].blocks[j]->stages[k].uniforms[l][m].type, "mat4")) {
								fprintf(render_header_file, "		M4 %s;\n", block_groups[i].blocks[j]->stages[k].uniforms[l][m].name);
							} else {
								Abort("unknown uniform type: %s", block_groups[i].blocks[j]->stages[k].uniforms[l][m].type);
							}
						}
					}
				}
			}
			fprintf(render_header_file, "	} %s;\n", shader_names[i]);
		}
		fprintf(render_header_file, "} Descriptor_Set_Parameters;\n\n");

		fprintf(render_header_file, "typedef struct Descriptor_Set_Data  {\n"
		                            "	GPU_Shader_ID shader_id;\n"
		                            "	s32 count;\n"
		                            "	Descriptor_Set_Parameters parameters;\n"
		                            "} Descriptor_Set_Data;\n\n");
	}
}

s32 main(s32 argc, char **argv) {
	static Material materials[MAX_MATERIALS];
	s32 material_count = Parse_Materials(materials);
	static char *shader_names[MAX_SHADERS];
	static Shader_Block_Group shader_block_groups[MAX_SHADERS];
	s32 shader_count = Generate_Shader_Code(material_count, materials, shader_names, shader_block_groups);
	Compile_Shader_Code(shader_count, shader_names, shader_block_groups);
	Generate_Render_Code(shader_count, shader_names, shader_block_groups);
}
