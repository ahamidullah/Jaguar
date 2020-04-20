#include "Basic/Basic.h"

bool CompileShader(const String &shaderFilepath)
{
	bool error;

	if (!FileExists(shaderFilepath))
	{
		LogPrint(ERROR_LOG, "Shader file %s does not exist\n", &shaderFilepath[0]);
		return false;
	}

	auto fileExtension = GetFilepathExtension(shaderFilepath);
	auto shaderName = GetFilepathFilename(shaderFilepath);
	SetFilepathExtension(&shaderName, "");

	// Write out a new version of the shader, inserting the includes.
	auto outputFilepath = FormatString("Build/Shader/Code/%s.%s", &shaderName[0], &fileExtension[1]);
	auto outputFile = OpenFile(outputFilepath, OPEN_FILE_WRITE_ONLY | OPEN_FILE_CREATE, &error);
	if (error)
	{
		LogPrint(ERROR_LOG, "Failed to open shader output file %s\n", &outputFilepath[0]);
		return false;
	}

	ParserStream parser = CreateParserStream(shaderFilepath, &error);
	if (error)
	{
		LogPrint(ERROR_LOG, "Failed to create parser\n");
		return false;
	}
	String line;
	while (GetLine(&parser, &line))
	{
		ParserStream lineParser =
		{
			.string = line,
		};
		auto token = GetToken(&lineParser);
		if (token == "#include")
		{
			GetExpectedToken(&lineParser, "\"");
			String includeFilepath;
			for (token = GetToken(&lineParser); token != "\""; token = GetToken(&lineParser))
			{
				StringAppend(&includeFilepath, token);
			}
			includeFilepath = JoinFilepaths("Code/Shader", includeFilepath);
			auto [includeString, error] = ReadEntireFile(includeFilepath);
			if (error)
			{
				LogPrint(ERROR_LOG, "Failed to read file %s included from %s\n", &includeFilepath[0], &shaderFilepath[0]);
				return false;
			}
			WriteStringToFile(outputFile, includeString);
		}
		else
		{
			WriteStringToFile(outputFile, line);
		}
	}

	auto command = FormatString("$VULKAN_SDK_PATH/bin/glslangValidator -V %s -S %s -o Build/Shader/Binary/%s.%s.spirv", &outputFilepath[0], &fileExtension[1], &shaderName[0], &fileExtension[1]);
	if (RunProcess(command) != 0)
	{
		LogPrint(ERROR_LOG, "Shader compilation command failed: %s", command);
		return false;
	}

	return true;
}

s32 ApplicationEntry(s32 argc, char *argv[])
{
	String shaderBuildDirectory = "Build/Shader";

	if (argc > 1)
	{
		CompileShader(argv[1]);
	}
	else
	{
		String shaderDirectory = "Code/Shader";
		DirectoryIteration iteration;
		while (IterateDirectory(shaderDirectory, &iteration))
		{
			auto shaderFilepath = JoinFilepaths(shaderDirectory, iteration.filename);
			auto fileExtension = GetFilepathExtension(shaderFilepath);
			if (fileExtension != ".vert" && fileExtension != ".frag" && fileExtension != ".comp" )
			{
				continue;
			}
			LogPrint(INFO_LOG, "-------------------------------------------------------------------------------------------------\n");
			LogPrint(INFO_LOG, "Compiling shader %s\n", &shaderFilepath[0]);
			if (CompileShader(shaderFilepath))
			{
				LogPrint(INFO_LOG, "Compiliation successful.\n");
			}
			else
			{
				LogPrint(INFO_LOG, "Compiliation failed.\n");
			}
		}
		LogPrint(INFO_LOG, "-------------------------------------------------------------------------------------------------\n");
	}

	return 1;
}
