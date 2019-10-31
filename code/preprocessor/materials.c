#define MAX_SHADER_INPUTS 32
#define MAX_SHADER_OUTPUTS 32
#define MAX_SHADER_UNIFORMS 32
#define MAX_SHADER_BLOCKS 1024
#define MAX_FILEPATH_LENGTH 256
#define MAX_MATERIALS 256
#define MAX_SHADERS 512

typedef enum Coordinate_System {
	INVALID_COORDINATE_SYSTEM,
	WORLD_COORDINATE_SYSTEM,
} Coordinate_System;

typedef struct Material {
	char name[256];
	bool has_color;
	V4 color;
	Coordinate_System coordinate_system;
} Material;

typedef struct Shader_Resource {
	char type[64];
	char name[64];
} Shader_Resource;

typedef struct Shader_Stage_Block {
	u32 uniform_count;
	Shader_Resource uniforms[MAX_SHADER_UNIFORMS];
	u32 input_count;
	Shader_Resource inputs[MAX_SHADER_INPUTS];
	u32 output_count;
	Shader_Resource outputs[MAX_SHADER_OUTPUTS];
	char main[256];
} Shader_Stage_Block;

typedef struct Shader_Block {
	Shader_Stage_Block vertex;
	Shader_Stage_Block fragment;
} Shader_Block;

// @TEMP
void Copy_String_T(Token source, char *destination) {
	for (u32 i = 0; i < source.length; i++) {
		destination[i] = source.string[i];
	}
}

#define STRING_VIEW_ERROR (String_View){}

typedef struct String_View {
	char *data;
	u32 length;
} String_View;

bool String_View_Equals_String(String_View a, String b) {
	if (a.length != b.length) {
		return false;
	}
	for (u32 i = 0; i < a.length; i++) {
		if (a.data[i] != b.data[i]) {
			return false;
		}
	}
	return true;
}

String_View Get_File_Extension(String filepath) {
	s64 dot_index = Find_Last_Occurrence_Of_Character(filepath, '.');
	if (dot_index < 0) {
		return STRING_VIEW_ERROR;
	}
	return (String_View){
		.data = filepath.data,
		.length = filepath.length - dot_index,
	};
}

s32 Parse_Materials(Material *materials) {
	String materials_directory = S("data/materials");
	s32 material_count = 0;
	Token token;
	Directory_Iteration materials_directory_iteration = {};
	while (Iterate_Through_All_Files_In_Directory(materials_directory, &materials_directory_iteration)) {
		materials[material_count] = (Material){};
		//strcpy(materials[*material_count].name, materials_directory_iteration.filename);
		String material_subdirectory = Join_Filepaths(materials_directory, materials_directory_iteration.filename);
		Directory_Iteration material_subdirectory_iteration = {};
		while (Iterate_Through_All_Files_In_Directory(material_subdirectory, &material_subdirectory_iteration)) {
			//const char *file_extension = Find_Last_Occurrence_Of_Character(material_subdirectory_iteration.filename, '.');
			//if (strcmp(file_extension, ".mat")) {
				//continue;
			//}
			if (!String_View_Equals_String(Get_File_Extension(material_subdirectory_iteration.filename), S(".mat"))) {
				continue;
			}
			static char material_file_contents[512 * 512];
			Stream stream = {
				.string = material_file_contents,
			};
			String material_file_path = Join_Filepaths(material_subdirectory, material_subdirectory_iteration.filename);
			Read_Entire_File(material_file_path, material_file_contents);
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
			Assert(material_count < MAX_MATERIALS);
		}
	}
	return material_count;
}

s32 Parse_Shader_Blocks(Shader_Block *shader_blocks) {
	String shader_block_directory = S("code/shader_blocks");
	Directory_Iteration directory_iteration = {};
	Token token;
	static char file_contents[512 * 512];
	s32 shader_block_count = 0;
	Shader_Stage_Block *current_shader_stage_block = NULL;
	while (Iterate_Through_All_Files_In_Directory(shader_block_directory, &directory_iteration)) {
		//const char *file_extension = Find_Last_Occurrence_Of_Character(directory_iteration.filename, '.');
		//if (strcmp(file_extension, ".glsl")) {
			//continue;
		//}
		if (!String_View_Equals_String(Get_File_Extension(directory_iteration.filename), S(".glsl"))) {
			continue;
		}
		String shader_block_filepath = Join_Filepaths(shader_block_directory, directory_iteration.filename);
		Read_Entire_File(shader_block_filepath, file_contents);
		Stream stream = {
			.string = file_contents,
		};
		while (Try_To_Get_Token(&stream, &token)) {
			if (Token_Equals(token, "@VERTEX") == 0) {
				current_shader_stage_block = &shader_blocks[shader_block_count].vertex;
				continue;
			} else if (Token_Equals(token, "@FRAGMENT") == 0) {
				current_shader_stage_block = &shader_blocks[shader_block_count].fragment;
				continue;
			}
			Assert(current_shader_stage_block);
			if (Token_Equals(token, "UNIFORM") == 0) {
				Get_Expected_Token(&stream, ":");
				Get_Expected_Token(&stream, "{");
				while (Peek_At_Next_Character(&stream) != '}') {
					Get_Token_Into_Buffer(&stream, current_shader_stage_block->uniforms[current_shader_stage_block->uniform_count].type);
					Get_Token_Into_Buffer(&stream, current_shader_stage_block->uniforms[current_shader_stage_block->uniform_count].name);
					Get_Expected_Token(&stream, ";");
					current_shader_stage_block->uniform_count++;
				}
				Get_Expected_Token(&stream, "}");
			} else if (Token_Equals(token, "INPUT") == 0) {
				Get_Expected_Token(&stream, ":");
				Get_Expected_Token(&stream, "{");
				while (Peek_At_Next_Character(&stream) != '}') {
					Get_Token_Into_Buffer(&stream, current_shader_stage_block->inputs[current_shader_stage_block->input_count].type);
					Get_Token_Into_Buffer(&stream, current_shader_stage_block->inputs[current_shader_stage_block->input_count].name);
					Get_Expected_Token(&stream, ";");
					current_shader_stage_block->input_count++;
				}
				Get_Expected_Token(&stream, "}");
			} else if (Token_Equals(token, "OUTPUT") == 0) {
				Get_Expected_Token(&stream, ":");
				Get_Expected_Token(&stream, "{");
				while (Peek_At_Next_Character(&stream) != '}') {
					Get_Token_Into_Buffer(&stream, current_shader_stage_block->outputs[current_shader_stage_block->output_count].type);
					Get_Token_Into_Buffer(&stream, current_shader_stage_block->outputs[current_shader_stage_block->output_count].name);
					Get_Expected_Token(&stream, ";");
					current_shader_stage_block->output_count++;
				}
				Get_Expected_Token(&stream, "}");
			} else if (Token_Equals(token, "MAIN") == 0) {
				Get_Expected_Token(&stream, ":");
				Token main_token = Get_Everything_In_Braces(&stream); // @TODO
				Copy_String_T(main_token, current_shader_stage_block->main);
			}
		}
		shader_block_count++;
		Assert(*shader_block_count < MAX_SHADER_BLOCKS);
	}
	return shader_block_count;
}

s32 Generate_Shaders(s32 material_count, Material *materials, char shader_filepaths[MAX_SHADERS][MAX_FILEPATH_LENGTH]) {
	static Shader_Block shader_blocks[MAX_SHADER_BLOCKS];
	s32 shader_block_count = Parse_Shader_Blocks(shader_blocks);
	return 0;
#if 0
	const char *shader_output_directory = "build/shaders/code";
	for (s32 i = 0; i < material_count; i++) {
		if (materials[i].has_color) {
		}
		switch (materials[i].coordinate_system) {
		case WORLD_COORDINATE_SYSTEM: {
		} break;
		case INVALID_COORDINATE_SYSTEM: {
			Abort("No coordinate assigned to material %s", materials[i].name);
		} break;
		}
	}
#endif
}

void Compile_Shaders(s32 shader_count) {
}

void Generate_Render_Code() {
}

void Compile_Materials() {
	static Material materials[MAX_MATERIALS];
	s32 material_count = Parse_Materials(materials);
	static char shader_filepaths[MAX_SHADERS][MAX_FILEPATH_LENGTH];
	s32 shader_count = Generate_Shaders(material_count, materials, shader_filepaths);
	Compile_Shaders(shader_count);
	Generate_Render_Code();
}
