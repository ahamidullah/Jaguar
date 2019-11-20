// @TODO: Shouldn't world space be world coordinate system?
// @TODO: Enum to string.

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
			if (*stream->string == '\t') {
				*buffer++ = '\t';
			}
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

#define MAX_SHADER_INPUTS 32
#define MAX_SHADER_OUTPUTS 32
#define MAX_SHADER_UNIFORMS 32
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

typedef struct Shader_Input_Output_Resource {
	char type[64];
	char name[64];
} Shader_Input_Output_Resource;

typedef enum Uniform_Type {
	UNIFORM_BUFFER_UNIFORM,
	DYNMAIC_UNIFORM_BUFFER_UNIFORM,
	SAMPLER_UNIFORM,
	COMBINED_IMAGE_SAMPLER_UNIFORM,
	SAMPLED_IMAGE_UNIFORM,

	UNIFORM_TYPE_COUNT
} Uniform_Type;

typedef enum Uniform_Set {
	PER_MATERIAL_UNIFORM_SET,
	PER_OBJECT_UNIFORM_SET,

	UNIFORM_SET_COUNT
} Uniform_Set;

typedef struct Shader_Uniform_Resource {
	Uniform_Set set;
	char type[64];
	char name[64];
} Shader_Uniform_Resource;

typedef struct Shader_Block_Resources {
	u32 uniform_count;
	Shader_Uniform_Resource uniforms[MAX_SHADER_UNIFORMS];

	u32 input_count;
	Shader_Input_Output_Resource inputs[MAX_SHADER_INPUTS];

	u32 output_count;
	Shader_Input_Output_Resource outputs[MAX_SHADER_OUTPUTS];
} Shader_Block_Resources;

typedef struct Shader_Block_Stage {
	Shader_Block_Resources resources;
	char main[MAX_SHADER_BLOCK_MAIN_LENGTH];
} Shader_Block_Stage;

typedef struct Shader_Block {
	char name[256];
	bool stages_used[SHADER_STAGE_COUNT];
	Shader_Block_Stage stages[SHADER_STAGE_COUNT];
} Shader_Block;

typedef struct Shader_Block_Group {
	//char *name;
	s32 block_count;
	Shader_Block *blocks[MAX_BLOCKS_PER_SHADER];
	//char *block_names[SHADER_STAGE_COUNT][MAX_BLOCKS_PER_SHADER];
	//Shader_Block_Resources *block_resources[SHADER_STAGE_COUNT][MAX_BLOCKS_PER_SHADER];
	//char *block_mains[SHADER_STAGE_COUNT][MAX_BLOCKS_PER_SHADER];
	//bool stages_used[SHADER_STAGE_COUNT];
} Shader_Block_Group;

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

s32 Parse_Materials(Material *materials) {
	const char *materials_directory = "data/materials";
	s32 material_count = 0;
	Token token;
	Directory_Iteration materials_directory_iteration = {};
	while (Iterate_Through_All_Files_In_Directory(materials_directory, &materials_directory_iteration)) {
		materials[material_count] = (Material){};
		//strcpy(materials[*material_count].name, materials_directory_iteration.filename);
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

typedef enum Shader_Block_ID {
	COLOR_SHADER_BLOCK,
	WORLD_SPACE_SHADER_BLOCK,
	
	SHADER_BLOCK_ID_COUNT,
} Shader_Block_ID;

#define Array_Count(x) (sizeof(x) / sizeof(x[0]))

s32 Parse_Shader_Blocks(Shader_Block *shader_blocks) {
	const char *shader_block_names[] = {
		[COLOR_SHADER_BLOCK] = "flat_color",
		[WORLD_SPACE_SHADER_BLOCK] = "world_space",
	};
	assert(Array_Count(shader_block_names) == SHADER_BLOCK_ID_COUNT);

	Token token;
	static char file_contents[512 * 512];
	s32 shader_block_count = 0;
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

		strcpy(shader_blocks[shader_block_count].name, shader_block_names[i]);

		while (Try_To_Get_Token(&stream, &token)) {
			if (Token_Equals(token, "@VERTEX")) {
				shader_blocks[shader_block_count].stages_used[VERTEX_SHADER_STAGE] = true;
				shader_block_stage = &shader_blocks[shader_block_count].stages[VERTEX_SHADER_STAGE];
				continue;
			} else if (Token_Equals(token, "@FRAGMENT")) {
				shader_blocks[shader_block_count].stages_used[FRAGMENT_SHADER_STAGE] = true;
				shader_block_stage = &shader_blocks[shader_block_count].stages[FRAGMENT_SHADER_STAGE];
				continue;
			}
			assert(shader_block_stage);
			if (Token_Equals(token, "UNIFORMS")) {
				Get_Expected_Token(&stream, ":");
				Get_Expected_Token(&stream, "{");
				while (Peek_At_Next_Character(&stream) != '}') {
					Get_Token_Into_Buffer(&stream, shader_block_stage->resources.uniforms[shader_block_stage->resources.uniform_count].type);
					Get_Token_Into_Buffer(&stream, shader_block_stage->resources.uniforms[shader_block_stage->resources.uniform_count].name);
					Get_Expected_Token(&stream, ";");
					token = Get_Token(&stream);
					Uniform_Set uniform_set;
					if (Token_Equals(token, "@PER_MATERIAL")) {
						uniform_set = PER_MATERIAL_UNIFORM_SET;
					} else if (Token_Equals(token, "@PER_OBJECT")) {
						uniform_set = PER_OBJECT_UNIFORM_SET;
					} else {
						Abort(Stream_Error(&stream, "expected a uniform set, got %.*s", token.length, token.string));
					}
					shader_block_stage->resources.uniforms[shader_block_stage->resources.uniform_count++].set = uniform_set;
				}
				Get_Expected_Token(&stream, "}");
			} else if (Token_Equals(token, "INPUT")) {
				Get_Expected_Token(&stream, ":");
				Get_Expected_Token(&stream, "{");
				while (Peek_At_Next_Character(&stream) != '}') {
					Get_Token_Into_Buffer(&stream, shader_block_stage->resources.inputs[shader_block_stage->resources.input_count].type);
					Get_Token_Into_Buffer(&stream, shader_block_stage->resources.inputs[shader_block_stage->resources.input_count].name);
					Get_Expected_Token(&stream, ";");
					shader_block_stage->resources.input_count++;
				}
				Get_Expected_Token(&stream, "}");
			} else if (Token_Equals(token, "OUTPUT")) {
				Get_Expected_Token(&stream, ":");
				Get_Expected_Token(&stream, "{");
				while (Peek_At_Next_Character(&stream) != '}') {
					Get_Token_Into_Buffer(&stream, shader_block_stage->resources.outputs[shader_block_stage->resources.output_count].type);
					Get_Token_Into_Buffer(&stream, shader_block_stage->resources.outputs[shader_block_stage->resources.output_count].name);
					Get_Expected_Token(&stream, ";");
					shader_block_stage->resources.output_count++;
				}
				Get_Expected_Token(&stream, "}");
			} else if (Token_Equals(token, "MAIN")) {
				Get_Expected_Token(&stream, ":");
				Get_Main_Block(&stream, shader_block_stage->main, sizeof(shader_block_stage->main));
			} else {
				Abort(Stream_Error(&stream, "Unknown shader block field: %.*s", token.length, token.string));
			}
		}
		shader_block_count++;
		assert(shader_block_count < MAX_SHADER_BLOCKS);
	}
	return shader_block_count;
}

#define MAX_UNIFORMS_PER_SET 16
#define MAX_INPUTS_PER_SHADER 16
#define MAX_OUTPUTS_PER_SHADER 16

typedef struct Shader_Uniforms {
	//s32 set_count;
	//Uniform_Set sets[UNIFORM_SET_COUNT];
	//Shader_Stage stages[UNIFORM_SET_COUNT][MAX_UNIFORMS_PER_SHADER];
	//Descriptor_Type types[UNIFORM_SET_COUNT][MAX_UNIFORMS_PER_SHADER];
} Shader_Uniforms;

typedef struct Shader_Inputs {
} Shader_Inputs;

typedef struct Shader_Outputs {
} Shader_Outputs;

typedef struct Shader_Resources {
	Shader_Uniforms descriptor_sets;
	Shader_Inputs inputs;
	Shader_Outputs outputs;
} Shader_Resources;

const char *SHADER_CODE_DIRECTORY = "build/shaders/code";
const char *SHADER_BINARY_DIRECTORY = "build/shaders/binaries";

void Output_Shader(const char *shader_name, Shader_Block_Group *block_group, bool stages_used[SHADER_STAGE_COUNT], Shader_Resources *shader_resources) {
	typedef struct Chunk_Builder {
		char buffer[512];
		char *write_pointer;
	} Chunk_Builder;

	struct {
		Chunk_Builder input;
		Chunk_Builder output;
		s32 standalone_uniform_counts[UNIFORM_SET_COUNT];
		Chunk_Builder standalone_uniforms[UNIFORM_SET_COUNT][MAX_UNIFORMS_PER_SET];
		Chunk_Builder uniform_blocks[UNIFORM_SET_COUNT];
		Chunk_Builder main;
	} source_chunks[SHADER_STAGE_COUNT] = {};

	for (s32 i = 0; i < SHADER_STAGE_COUNT; i++) {
		source_chunks[i].input.write_pointer = source_chunks[i].input.buffer;
		source_chunks[i].output.write_pointer = source_chunks[i].output.buffer;
		source_chunks[i].main.write_pointer = source_chunks[i].main.buffer;
		for (s32 j = 0; j < UNIFORM_SET_COUNT; j++) {
			source_chunks[i].uniform_blocks[j].write_pointer = source_chunks[i].uniform_blocks[j].buffer;
			for (s32 k = 0; k < MAX_UNIFORMS_PER_SET; k++) {
				source_chunks[i].standalone_uniforms[j][k].write_pointer = source_chunks[i].standalone_uniforms[j][k].buffer;
			}
		}
	}

	s32 stage_input_counts[SHADER_STAGE_COUNT] = {};
	s32 stage_output_counts[SHADER_STAGE_COUNT] = {};
	for (s32 i = 0; i < block_group->block_count; i++) {
		for (s32 j = 0; j < SHADER_STAGE_COUNT; j++) {
			if (!block_group->blocks[i]->stages_used[j]) {
				continue;
			}
			Shader_Block_Resources *block_resources = &block_group->blocks[i]->stages[j].resources;
			if (block_resources->input_count) {
				for (s32 k = 0; k < block_resources->input_count; k++) {
					source_chunks[j].input.write_pointer += sprintf(source_chunks[j].input.write_pointer, "layout (location = %d) in %s %s; // %s\n", stage_input_counts[j], block_resources->inputs[k].type, block_resources->inputs[k].name, block_group->blocks[i]->name);
					stage_input_counts[j]++;
				}
				source_chunks[j].input.write_pointer += sprintf(source_chunks[j].input.write_pointer, "\n");
			}
			if (block_resources->output_count) {
				for (s32 k = 0; k < block_resources->output_count; k++) {
					source_chunks[j].output.write_pointer += sprintf(source_chunks[j].output.write_pointer, "layout (location = %d) out %s %s; // %s\n", stage_output_counts[j], block_resources->outputs[k].type, block_resources->outputs[k].name, block_group->blocks[i]->name);
					stage_output_counts[j]++;
				}
				source_chunks[j].output.write_pointer += sprintf(source_chunks[j].output.write_pointer, "\n");
			}
			if (block_resources->uniform_count) {
				for (s32 k = 0; k < block_resources->uniform_count; k++) {
					Uniform_Set set = block_resources->uniforms[k].set;
					if (Strings_Equal(block_resources->uniforms[k].type, "texture2D")) {
						u32 index = source_chunks[j].standalone_uniform_counts[set];
						source_chunks[j].standalone_uniforms[set][index].write_pointer += sprintf(source_chunks[j].standalone_uniforms[set][index].write_pointer, "%s %s; // %s", block_resources->uniforms[k].type, block_resources->uniforms[k].name, block_group->blocks[i]->name);
						source_chunks[j].standalone_uniform_counts[set]++;
					} else {
						source_chunks[j].uniform_blocks[set].write_pointer += sprintf(source_chunks[j].uniform_blocks[set].write_pointer, "\t%s %s; // %s\n", block_resources->uniforms[k].type, block_resources->uniforms[k].name, block_group->blocks[i]->name);
					}
					//shader_resources->descriptor_sets.stages[shader_resources->descriptor_sets.count] = j;
					//shader_resources->descriptor_sets.count++;
				}
			}
			if (source_chunks[j].main.write_pointer != source_chunks[j].main.buffer) {
				*source_chunks[j].main.write_pointer++ = '\n';
			}
			source_chunks[j].main.write_pointer += sprintf(source_chunks[j].main.write_pointer, "\t// %s\n", block_group->blocks[i]->name);
			source_chunks[j].main.write_pointer += sprintf(source_chunks[j].main.write_pointer, "\t{\n");
			source_chunks[j].main.write_pointer += sprintf(source_chunks[j].main.write_pointer, "%s", block_group->blocks[i]->stages[j].main);
			source_chunks[j].main.write_pointer += sprintf(source_chunks[j].main.write_pointer, "\t}\n");
		}
	}

	static char shader_filepath[MAX_FILEPATH_LENGTH];
	sprintf(shader_filepath, "%s/%s.glsl", SHADER_CODE_DIRECTORY, shader_name);
	FILE *shader_file = fopen(shader_filepath, "w");
	fprintf(shader_file, "// This file was auto-generated by material_compiler.c.\n\n");
	fprintf(shader_file, "#version 420\n\n");
	for (s32 i = 0; i < SHADER_STAGE_COUNT; i++) {
		// @TODO: Store a list of used stages rather than a bool array.
		if (!stages_used[i]) {
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

		fprintf(shader_file, "%s", source_chunks[i].input.buffer);
		fprintf(shader_file, "%s", source_chunks[i].output.buffer);

		for (s32 j = 0; j < UNIFORM_SET_COUNT; j++) {
			if (!source_chunks[i].uniform_blocks[j].buffer[0]) {
				continue;
			}
			s32 binding_number = 0;
			char *ubo_name;
			switch (j) {
			case PER_MATERIAL_UNIFORM_SET: {
				ubo_name = "Per_Material_UBO";
			} break;
			case PER_OBJECT_UNIFORM_SET: {
				ubo_name = "Per_Object_UBO";
			} break;
			default: {
				Abort("unknown uniform update frequency");
			} break;
			}
			fprintf(shader_file, "layout (set = %d, binding = %d) uniform %s {\n", j, binding_number, ubo_name);
			fprintf(shader_file, "%s", source_chunks[i].uniform_blocks[j].buffer);
			fprintf(shader_file, "};\n");
			binding_number++;

			for (s32 k = 0; k < source_chunks[i].standalone_uniform_counts[j]; k++) {
				fprintf(shader_file, "layout (set = %d, binding = %d) uniform %s\n", j, binding_number, source_chunks[i].standalone_uniforms[j][k].buffer);
				binding_number++;
			}
			fprintf(shader_file, "\n");
		}

		fprintf(shader_file, "void main() {\n");
		fprintf(shader_file, "%s", source_chunks[i].main.buffer);
		fprintf(shader_file, "}\n\n");
		fprintf(shader_file, "#endif\n");
	}
	fclose(shader_file);
}

void Copy_Shader_Block_To_Group(Shader_Block *block, Shader_Block_Group *block_group, bool stages_used[SHADER_STAGE_COUNT]) {
	block_group->blocks[block_group->block_count++] = block;
	assert(block_group->block_count < MAX_BLOCKS_PER_SHADER);
	for (s32 i = 0; i < SHADER_STAGE_COUNT; i++) {
		if (block->stages_used[i]) {
			stages_used[i] = true;
		}
	}
}

s32 Generate_Shader_Code(s32 material_count, Material *materials, char *shader_names[MAX_SHADERS], bool stages_used[MAX_SHADERS][SHADER_STAGE_COUNT], Shader_Resources shader_resources[MAX_SHADERS]) {
	static Shader_Block shader_blocks[MAX_SHADER_BLOCKS];
	s32 shader_block_count = Parse_Shader_Blocks(shader_blocks);
	s32 shader_count = 0;
	for (s32 i = 0; i < material_count; i++) {
		Shader_Block_Group block_group = {};
		if (materials[i].has_color) {
			Copy_Shader_Block_To_Group(&shader_blocks[COLOR_SHADER_BLOCK], &block_group, stages_used[shader_count]);
		}
		switch (materials[i].coordinate_system) {
		case WORLD_COORDINATE_SYSTEM: {
			Copy_Shader_Block_To_Group(&shader_blocks[WORLD_SPACE_SHADER_BLOCK], &block_group, stages_used[shader_count]);
		} break;
		case INVALID_COORDINATE_SYSTEM: {
			Abort("No coordinate system assigned to material %s", materials[i].name);
		} break;
		}
		assert(block_group.block_count < MAX_BLOCKS_PER_SHADER);
		shader_names[i] = materials[i].name;
		Output_Shader(shader_names[shader_count], &block_group, stages_used[shader_count], &shader_resources[shader_count]);
		shader_count++;
	}
	assert(shader_count < MAX_SHADERS);
	return shader_count;
}

void Compile_Shader_Code(s32 shader_count, char *shader_names[MAX_SHADERS], bool stages_used[MAX_SHADERS][SHADER_STAGE_COUNT]) {
	for (s32 i = 0; i < shader_count; i++) {
		static char command[256];
		for (s32 j = 0; j < SHADER_STAGE_COUNT; j++) {
			if (!stages_used[i][j]) {
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

const char *RENDER_CODE_SOURCE_FILEPATH = "code/vulkan_generated.c";
const char *RENDER_CODE_HEADER_FILEPATH = "code/vulkan_generated.h";

void Uppercase_String(const char *source, char *destination) {
	while (*source) {
		*destination++ = toupper(*source++);
	}
	*destination = '\0';
}

void Generate_Render_Code(s32 shader_count, char *shader_names[MAX_SHADERS], Shader_Resources shader_resources[MAX_SHADERS], bool stages_used[MAX_SHADERS][SHADER_STAGE_COUNT]) {
	FILE *source_file = fopen(RENDER_CODE_SOURCE_FILEPATH, "w");
	FILE *header_file = fopen(RENDER_CODE_HEADER_FILEPATH, "w");
	assert(source_file && header_file);

	static char uppercase_shader_names[MAX_SHADERS][256];
	for (s32 i = 0; i < shader_count; i++) {
		Uppercase_String(shader_names[i], uppercase_shader_names[i]);
	}

	// Header file.
	{
		fprintf(header_file, "// This file was auto-generated by material_compiler.c.\n\n");

		fprintf(header_file, "typedef enum GPU_Shader_ID {\n");
		for (s32 i = 0; i < shader_count; i++) {
			fprintf(header_file, "	%s_SHADER,\n", uppercase_shader_names[i]);
		}
		fprintf(header_file, "\n	SHADER_COUNT,\n");
		fprintf(header_file, "} GPU_Shader_ID;\n\n");

		fprintf(header_file, "typedef enum GPU_Shader_Stage {\n"
							 "	VERTEX_SHADER_STAGE,\n"
							 "	FRAGMENT_SHADER_STAGE,\n"
							 "	SHADER_STAGE_COUNT,\n"
							 "} GPU_Shader_Stage;\n\n");

		fprintf(header_file, "typedef struct GPU_Shader {\n"
							 "	s32 module_count;\n"
							 "	VkShaderModule modules[SHADER_STAGE_COUNT];\n"
							 "} GPU_Shader;\n\n");

		static char uppercase_descriptor_set_name[256];
		fprintf(header_file, "typedef enum GPU_Descriptor_Set_ID {\n");
		for (s32 i = 0; i < shader_count; i++) {
			//for (s32 j = 0; j < shader_resources[i].descriptor_sets.count; j++) {
				//Uppercase_String(shader_resources[i].descriptor_sets.names[j], uppercase_descriptor_set_name);
				//fprintf(header_file, "	%s_DESCRIPTOR_SET,\n", uppercase_descriptor_set_name);
			//}
		}
		fprintf(header_file, "} GPU_Descriptor_Set_ID;\n\n");

		for (s32 i = 0; i < shader_count; i++) {
			//fprintf(header_file, "#define %s_DESCRIPTOR_SET_COUNT %d\n", uppercase_shader_names[i], shader_resources[i].descriptor_sets.count);
		}
		fprintf(header_file, "\n");

		fprintf(header_file, "typedef struct Descriptor_Set_Layouts {\n");
		for (s32 i = 0; i < shader_count; i++) {
			fprintf(header_file, "	VkDescriptorSetLayout %s[%s_DESCRIPTOR_SET_COUNT];\n", shader_names[i], uppercase_shader_names[i]);
		}
		fprintf(header_file, "} Descriptor_Set_Layouts;\n\n");
	}

	// Source file.
	{
		fprintf(source_file, "// This file was auto-generated by material_compiler.c.\n\n");

		fprintf(source_file, "const char *SHADER_SPIRV_FILEPATHS[SHADER_COUNT][SHADER_STAGE_COUNT] = {\n");
		for (s32 i = 0; i < shader_count; i++) {
			for (s32 j = 0; j < SHADER_STAGE_COUNT; j++) {
				if (!stages_used[j]) {
					continue;
				}
				const char *stage_enum, *stage_name;
				switch (j) {
				case VERTEX_SHADER_STAGE: {
					stage_enum = "VERTEX_SHADER_STAGE";
					stage_name = "vert";
				} break;
				case FRAGMENT_SHADER_STAGE: {
					stage_enum = "FRAGMENT_SHADER_STAGE";
					stage_name = "frag";
				} break;
				default: {
					Abort("Unknown shader stage");
				} break;
				}
				fprintf(source_file, "	[%s_SHADER][%s] = \"%s/%s_%s.spirv\",\n", uppercase_shader_names[i], stage_enum, SHADER_BINARY_DIRECTORY, shader_names[i], stage_name);
			}
		}
		fprintf(source_file, "};\n\n");

		fprintf(source_file, "void Render_API_Load_Shaders(Render_API_Context *context, GPU_Shader shaders[SHADER_COUNT]) {\n"
							 "	for (s32 i = 0; i < SHADER_COUNT; i++) {\n"
							 "		for (s32 j = 0; j < SHADER_STAGE_COUNT; j++) {\n"
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
							 "			VK_CHECK(vkCreateShaderModule(context->device, &shader_module_create_info, NULL, &shaders[i].modules[shaders[i].module_count++]));\n"
							 "		}\n"
							 "	}\n"
							 "}\n\n");

		fprintf(source_file, "void Render_API_Create_Descriptor_Sets(Render_API_Context *context) {\n"
							 "}\n\n");
	}

	fclose(source_file);
	fclose(header_file);
}

s32 main(s32 argc, char **argv) {
	static Material materials[MAX_MATERIALS];
	s32 material_count = Parse_Materials(materials);
	static char *shader_names[MAX_SHADERS];
	static bool shader_stages_used[MAX_SHADERS][SHADER_STAGE_COUNT];
	static Shader_Resources shader_resources[MAX_SHADERS];
	s32 shader_count = Generate_Shader_Code(material_count, materials, shader_names, shader_stages_used, shader_resources);
	Compile_Shader_Code(shader_count, shader_names, shader_stages_used);
	Generate_Render_Code(shader_count, shader_names, shader_resources, shader_stages_used);
}
