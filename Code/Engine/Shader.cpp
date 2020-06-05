#include "Shader.h"

#include "Code/Basic/Parser.h"
#include "Code/Basic/File.h"

struct ShaderGlobals
{
	HashTable<String, Shader> shaders;
} shaderGlobals;

String CompileShader(const String &filepath, bool *error)
{
	if (!FileExists(filepath))
	{
		LogPrint(ERROR_LOG, "Shader file %k does not exist.\n", filepath);
		*error = true;
		return {};
	}

	CreateDirectoryIfItDoesNotExist("Build/Linux/Shader");
	CreateDirectoryIfItDoesNotExist("Build/Linux/Shader/Code");
	CreateDirectoryIfItDoesNotExist("Build/Linux/Shader/Binary");

	auto fileExtension = GetFilepathExtension(filepath);
	auto name = GetFilepathFilename(filepath);
	SetFilepathExtension(&name, "");

	// Write out a new version of the shader, inserting the text for the include files.
	auto processedFilepath = FormatString("Build/Linux/Shader/Code/%k%k", name, fileExtension);
	auto processedFile = OpenFile(processedFilepath, OPEN_FILE_WRITE_ONLY | OPEN_FILE_CREATE, error);
	if (*error)
	{
		LogPrint(ERROR_LOG, "Failed to open processed shader output file %k.\n", processedFilepath);
		*error = true;
		return {};
	}
	Defer(CloseFile(processedFile));

	auto parser = CreateParser(filepath, STANDARD_PARSER_DELIMITERS, error);
	if (*error)
	{
		LogPrint(ERROR_LOG, "Failed to create parser for shader %k.\n", filepath);
		*error = true;
		return {};
	}
	for (auto line = GetParserLine(&parser); line != ""; line = GetParserLine(&parser))
	{
		Parser lineParser =
		{
			.string = line,
			.delimiters = " \"\n",
		};
		auto token = GetParserToken(&lineParser);
		if (token != "#include")
		{
			WriteToFile(processedFile, StringLength(line), &line[0]);
			continue;
		}
		if (GetParserToken(&lineParser) != "\"")
		{
			LogPrint(ERROR_LOG, "Expected '\"' at %k:%ld:%ld.\n", filepath, parser.line, lineParser.column);
			*error = true;
			return {};
		}
		auto includeFilepath = JoinFilepaths("Code/Shader", GetParserToken(&lineParser));
		if (GetParserToken(&lineParser) != "\"")
		{
			LogPrint(ERROR_LOG, "Expected '\"' at %k:%ld:%ld.\n", filepath, parser.line, lineParser.column);
			*error = true;
			return {};
		}
		auto includeFileContents = ReadEntireFile(includeFilepath, error);
		if (*error)
		{
			LogPrint(ERROR_LOG, "Failed to read include file %k at %k:%ld%ld.\n", includeFilepath, filepath, parser.line, lineParser.column);
			*error = true;
			return {};
		}
		WriteToFile(processedFile, StringLength(includeFileContents), &includeFileContents[0]);
	}

	auto spirvFilepath = FormatString("Build/Linux/Shader/Binary/%k%k.spirv", name, fileExtension);
	auto command = FormatString("$VulkanSDKDirectory/bin/glslangValidator -V %k -o %k", processedFilepath, spirvFilepath);
	if (RunProcess(command) != 0)
	{
		LogPrint(ERROR_LOG, "\nShader compilation command failed: %k.\n", command);
		*error = true;
		return {};
	}
	auto spirv = ReadEntireFile(spirvFilepath, error);
	if (*error)
	{
		LogPrint(ERROR_LOG, "Failed to read spirv file %k.\n", spirvFilepath);
		*error = true;
		return {};
	}
	return spirv;
}

void LoadShaders()
{
	auto shaderDirectory = String{"Code/Shader"};
	auto iteration = DirectoryIteration{};
	while (IterateDirectory(shaderDirectory, &iteration))
	{
		auto filepath = JoinFilepaths(shaderDirectory, iteration.filename);
		auto fileExtension = GetFilepathExtension(filepath);
		auto stage = GfxShaderStage{};
		if (fileExtension == ".vert")
		{
			stage = GFX_VERTEX_SHADER_STAGE;
		}
		else if (fileExtension == ".frag")
		{
			stage = GFX_FRAGMENT_SHADER_STAGE;
		}
		else if (fileExtension == ".comp")
		{
			stage = GFX_COMPUTE_SHADER_STAGE;
		}
		else
		{
			continue;
		}

		LogPrint(INFO_LOG, "Loading shader %k.\n", filepath);

		auto error = false;
		auto spirv = CompileShader(filepath, &error);
		if (error)
		{
			LogPrint(ERROR_LOG, "Failed to compile shader %k.\n", filepath);
			continue;
		}
		auto name = CreateString(iteration.filename);
		SetFilepathExtension(&name, "");
		if (auto shader = GetShader(name); shader)
		{
			ArrayAppend(&shader->stages, stage);
			ArrayAppend(&shader->modules, GfxCreateShaderModule(stage, spirv));
		}
		else
		{
			auto newShader = Shader{.name = name};
			ArrayAppend(&newShader.stages, stage);
			ArrayAppend(&newShader.modules, GfxCreateShaderModule(stage, spirv));
			InsertIntoHashTable(&shaderGlobals.shaders, newShader.name, newShader);
		}
	}
}

Shader *GetShader(const String &name)
{
	auto shader = LookupInHashTable(&shaderGlobals.shaders, name);
	if (!shader)
	{
		return NULL;
	}
	return shader;
}
