#include "Basic/Basic.h"

bool CompileShader(const String &shaderFilepath)
{
	auto error = false;

	if (!FileExists(shaderFilepath))
	{
		LogPrint(ERROR_LOG, "Shader file %k does not exist.\n", shaderFilepath);
		return false;
	}

	auto fileExtension = GetFilepathExtension(shaderFilepath);
	auto shaderName = GetFilepathFilename(shaderFilepath);
	SetFilepathExtension(&shaderName, "");

	// Write out a new version of the shader, inserting the includes.
	auto outputFilepath = FormatString("Build/Shader/Code/%k.%s", shaderName, &fileExtension[1]);
	auto outputFile = OpenFile(outputFilepath, OPEN_FILE_WRITE_ONLY | OPEN_FILE_CREATE, &error);
	if (error)
	{
		LogPrint(ERROR_LOG, "Failed to open shader output file %k.\n", outputFilepath);
		return false;
	}

	auto parser = CreateParser(shaderFilepath, STANDARD_PARSER_DELIMITERS, &error);
	if (error)
	{
		LogPrint(ERROR_LOG, "Failed to create parser for shader %k.\n", shaderFilepath);
		return false;
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
			WriteToFile(outputFile, StringLength(line), &line[0]);
			continue;
		}
		if (GetParserToken(&lineParser) != "\"")
		{
			LogPrint(ERROR_LOG, "In file included from %k:%ld:%ld: Expected '\"'.\n", shaderFilepath, parser.line, lineParser.column);
			return false;
		}
		auto includeFilepath = JoinFilepaths("Code/Shader", GetParserToken(&lineParser));
		if (GetParserToken(&lineParser) != "\"")
		{
			LogPrint(ERROR_LOG, "In file included from %k:%ld:%ld: Expected '\"'.\n", shaderFilepath, parser.line, lineParser.column);
			return false;
		}
		auto includeFileContents = ReadEntireFile(includeFilepath, &error);
		if (error)
		{
			LogPrint(ERROR_LOG, "In file included from %k:%ld%ld: failed to read include file %k.\n", shaderFilepath, parser.line, lineParser.column, includeFilepath);
			return false;
		}
		WriteToFile(outputFile, StringLength(includeFileContents), &includeFileContents[0]);
	}

#if 0
		auto shaderFileContents = ParserGetUntilChar(&parser, '#');
		WriteToFile(outputFile, StringLength(shaderFileContents), &shaderFileContents[0]);

		auto token = GetParserToken(&parser);
		if (token != "#include")
		{
			WriteToFile(outputFile, StringLength(token), &token[0]);
			continue;
		}
		if (!GetIfParserToken(&parser, "\""))
		{
			LogPrint(ERROR_LOG, "%k:%ld%ld: Expected '\"'", shaderFilepath, parser.line, parser.column);
			return false;
		}
		auto includeFilepath = JoinFilepaths("Code/Shader", GetParserToken(&parser));
		if (!GetIfParserToken(&parser, "\""))
		{
			LogPrint(ERROR_LOG, "%k:%ld%ld: Expected '\"'", shaderFilepath, parser.line, parser.column);
			return false;
		}
		auto includeFileString = ReadEntireFile(includeFilepath, &error);
		if (error)
		{
			LogPrint(ERROR_LOG, "%k:%ld:%ld: failed to read include file %k.\n", shaderFilepath, parser.line, parser.column, includeFilepath);
			return false;
		}
		WriteToFile(outputFile, StringLength(includeFileString), &includeFileString[0]);
	}
#endif

#if 0
		if (token[0] == '#' && GetIfParserToken(&parser, " \"", "include"))
		{
			if (!GetIfParserToken(&parser, "\"", "\""))
			{
				continue;
			}
			auto includeFilepath = JoinFilepaths("Code/Shader", GetParserToken(&parser, "\""));
			auto includeFileString = ReadEntireFile(includeFilepath, &error);
			if (error)
			{
				LogPrint(ERROR_LOG, "%k:%ld:%ld: failed to read include file %k.\n", shaderFilepath, parser.lineCount, parser.lineCharCount, includeFilepath);
				return false;
			}
			WriteToFile(outputFile, StringLength(includeFileString), &includeFileString[0]);
			GetToken();
		}
		else
		{
			WriteToFile(outputFile, StringLength(token), &token[0]);
		}
	}
#endif

	auto command = FormatString("$VULKAN_SDK_PATH/bin/glslangValidator -V %s -S %s -o Build/Shader/Binary/%s.%s.spirv", &outputFilepath[0], &fileExtension[1], &shaderName[0], &fileExtension[1]);
	if (RunProcess(command) != 0)
	{
		LogPrint(ERROR_LOG, "\nShader compilation command failed: %k.\n", command);
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
			LogPrint(INFO_LOG, "Compiling shader %k...\n", shaderFilepath);
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
