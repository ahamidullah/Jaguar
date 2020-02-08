#include "Common.cpp"
#include "Strings.cpp"
#include "Array.h"

String ReadEntireFile(const String &path) {
	FILE *fileHandle = fopen(path, "r");
	if (!fileHandle) {
		Abort("Failed to open file %s: %s\n", path, strerror(errno));
	}
	Defer(fclose(fileHandle));
	fseek(fileHandle, 0L, SEEK_END);
	s64 fileLength = ftell(fileHandle);
	String result = CreateString(fileLength);
	fseek(fileHandle, 0L, SEEK_SET);
	size_t numberOfBytesRead = fread(&result[0], 1, fileLength, fileHandle);
	if (numberOfBytesRead != fileLength) {
		Abort("Failed to read file %s: %s\n", path, strerror(errno));
	}
	return result;
}

struct Stream {
	const String &string;
	String name;
	u32 line;
	u32 index;
};

void Insert(Stream *stream, const String &what) {
}

struct Token {
	char *string;
	u32 length;
	bool operator==(const char *comparand) {
		u32 i = 0;
		for (; (i < length) && comparand[i]; i++) {
			if (string[i] != comparand[i]) {
				return false;
			}
		}
		if (i != length || comparand[i] != '\0') {
			return false;
		}
		return true;
	}
};

enum ShaderStage {
	VERTEX_SHADER_STAGE,
	FRAGMENT_SHADER_STAGE,
	SHADER_STAGE_COUNT
}

s32 main(s32 argc, char *argv[]) {
	String shaderBinaryOutputDirectory = "build/Shaders/Binaries";
	String shaderProcessedCodeDirecotyr = "build/Shaders/Code";
	String shaderCodeDirectory = "code/Shaders";
	String shaderName = "FlatColor.glsl";
	String shaderPath = Concatenate(shaderCodeDirectory, "/", shaderName);
	Stream stream = {
		.string = ReadEntireFile(shaderPath),
		.name = shaderPath,
	};

	Array<ShaderStage> stagesUsed;
	Token token;
	while (TryToGetToken(&stream, &token) {
		if (token == "layout") {
			token = GetToken(&stream);
			if (token == "uniform") {
			} else if (token == "in") {
			} else if (token == "out") {
			}
		}
		if (token == "VERTEX_SHADER") {
			Append(&stagesUsed, VERTEX_SHADER_STAGE);
		}
		if (token == "FRAGMENT_SHADER") {
			Append(&stagesUsed, FRAGMENT_SHADER_STAGE);
		}
	}

	for (auto stage : stagesUsed) {
		const char *stageDefine;
		const char *stageID;
		switch (stage) {
		case VERTEX_SHADER_STAGE: {
			stageDefine = "VERTEX_SHADER";
			stageID = "vert";
		} break;
		case FRAGMENT_SHADER_STAGE: {
			stageDefine = "FRAGMENT_SHADER";
			stageID = "frag";
		} break;
		default: {
			Abort("unknown shader stage %d\n", stage);
		} break;
		}
		sprintf(command, "$VULKAN_SDK_PATH/bin/glslangValidator -V %s/%s.glsl -D%s -S %s -o %s/%s_%s.spirv", shaderCodeDirectory, shaderName, stageDefine, stageID, shaderBinaryOutputDirectory, shaderName, stageID);
		printf("%s\n", command);
		if (system(command) != 0) {
			Abort("Shader compilation command failed: %s", command);
		}
	}
}
