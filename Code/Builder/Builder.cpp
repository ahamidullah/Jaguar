#include "Basic/Basic.h"

bool debugBuild = true;
bool developmentBuild = true;

Array<String> includeSearchDirectories;

bool GatherDependencies(const String &sourceFilepath, Array<String> *dependencies)
{
	Array<String> queuedFiles;
	Array<String> finishedFiles;
	Append(&queuedFiles, sourceFilepath);

	for (auto i = 0; i < Length(queuedFiles); i++)
	{
		ParserStream parser;
		if (!CreateParserStream(&parser, queuedFiles[i]))
		{
			LogPrint(LogType::ERROR, "%s: failed to create parser\n", &queuedFiles[i][0]);
			return false;
		}

		Append(dependencies, queuedFiles[i]);

		auto directory = GetFilepathDirectory(queuedFiles[i]);

		bool inBlockComment = false;
		bool inLineComment = false;
		String token;
		for (token = GetToken(&parser); Length(token) > 0; token = GetToken(&parser))
		{
			if (inLineComment && token == "\n")
			{
				inLineComment = false;
			}
			else if (!inLineComment && !inBlockComment && token == "/")
			{
				auto nextChar = PeekAtNextCharacter(&parser);
				if (nextChar == '/')
				{
					inLineComment = true;
					AdvanceParser(&parser, 1);
				}
				else if (nextChar == '*')
				{
					inBlockComment = true;
					AdvanceParser(&parser, 1);
				}
			}
			else if (inBlockComment && token == "*")
			{
				auto nextChar = PeekAtNextCharacter(&parser);
				if (nextChar == '/')
				{
					inBlockComment = false;
					AdvanceParser(&parser, 1);
				}
			}
			else if (!inBlockComment && !inLineComment && token == "#include")
			{
				token = GetToken(&parser);
				if (token != "\"")
				{
					continue;
				}
				String includePartialFilepath;
				token = GetToken(&parser);
				while (Length(token) > 0 && token != "\"")
				{
					Append(&includePartialFilepath, token);
					token = GetToken(&parser);
				}
				if (Length(token) == 0)
				{
					LogPrint(LogType::ERROR, "%s: unexpected end of include filepath: %s\n", &includePartialFilepath[0]);
					return false;
				}
				bool alreadyDone = false;
				for (auto &f : finishedFiles)
				{
					if (f == includePartialFilepath)
					{
						alreadyDone = true;
						break;
					}
				}
				if (!alreadyDone)
				{
					bool foundFile = false;
					auto includeFilepath = JoinFilepaths(directory, includePartialFilepath);
					if (FileExists(includeFilepath))
					{
						foundFile = true;
					}
					else
					{
						for (auto &dir : includeSearchDirectories)
						{
							includeFilepath = JoinFilepaths(dir, includePartialFilepath);
							if (FileExists(includeFilepath))
							{
								foundFile = true;
								break;
							}
						}
					}
					if (!foundFile)
					{
						LogPrint(LogType::ERROR, "%s: failed to find include file %s\n", &queuedFiles[i][0], &includePartialFilepath[0]);
						return false;
					}
					Append(&queuedFiles, CleanFilepath(includeFilepath));
				}
			}
		}

		Append(&finishedFiles, queuedFiles[i]);
	}

	return true;
}

struct BuildCommand
{
	String sourceFilepath;
	String compilerFlags;
	String linkerFlags;
	String binaryFilepath;
	bool isLibrary;
};

bool Build(const BuildCommand &command)
{
	LogPrint(LogType::INFO, "sourceFilepath: %s\n\nisLibrary: %d\n\ncompilerFlags: %s\n\nlinkerFlags: %s\n\nbinaryFilepath: %s\n\n", &command.sourceFilepath[0], command.isLibrary, &command.compilerFlags[0], &command.linkerFlags[0], &command.binaryFilepath[0]);

	auto objFilepath = CreateString(command.binaryFilepath);
	SetFilepathExtension(&objFilepath, ".o");

	if (command.isLibrary)
	{
		auto compileCommand = _FormatString("g++ -shared %s %s %s -o %s", &command.compilerFlags[0], &command.sourceFilepath[0], &command.linkerFlags[0], &command.binaryFilepath[0]);
		LogPrint(LogType::INFO, "compileCommand: %s\n\n", &compileCommand[0]);
		if (auto result = RunProcess(compileCommand); result != 0)
		{
			return false;
		}
	}
	else
	{
		auto compileCommand = _FormatString("g++ %s %s %s -o %s", &command.compilerFlags[0], &command.sourceFilepath[0], &command.linkerFlags[0], &command.binaryFilepath[0]);
		LogPrint(LogType::INFO, "compileCommand: %s\n\n", &compileCommand[0]);
		if (auto result = RunProcess(compileCommand); result != 0)
		{
			return false;
		}
	}

	LogPrint(LogType::INFO, "build success\n");

	return true;
}

bool BuildIfOutOfDate(const BuildCommand &command)
{
	auto directory = GetFilepathDirectory(command.sourceFilepath);
	Assert(Length(directory) > 0);

	Array<String> dependencies;
	if (!GatherDependencies(command.sourceFilepath, &dependencies))
	{
		return false;
	}

	bool needsBuild = false;
	if (FileExists(command.binaryFilepath))
	{
		auto [binaryFile, binaryOpenError] = OpenFile(command.binaryFilepath, OPEN_FILE_READ_ONLY);
		Assert(!binaryOpenError);
		auto binaryLastModifiedTime = GetFileLastModifiedTime(binaryFile);

		for (auto &sourceFilename : dependencies)
		{
			auto [sourceFile, sourceOpenError] = OpenFile(sourceFilename, OPEN_FILE_READ_ONLY);
			Assert(!sourceOpenError);
			Defer(CloseFile(sourceFile));
			auto lastModifiedTime = GetFileLastModifiedTime(sourceFile);
			if (lastModifiedTime > binaryLastModifiedTime)
			{
				LogPrint(LogType::INFO, "%s is out of date, rebuilding...\n\n", &command.binaryFilepath[0]);
				needsBuild = true;
				break;
			}
		}

		if (!needsBuild)
		{
			LogPrint(LogType::INFO, "%s is up to date, skipping build\n", &command.binaryFilepath[0]);
		}
	}
	else
	{
		LogPrint(LogType::INFO, "%s does not exist, building...\n\n", &command.binaryFilepath[0]);
		needsBuild = true;
	}

	if (needsBuild)
	{
		return Build(command);
	}

	return true;
}

s32 ApplicationEntry(s32 argc, char *argv[])
{
	CreateDirectoryIfItDoesNotExist(JoinFilepaths(buildDirectory, "ShaderPreprocessor"));
	CreateDirectoryIfItDoesNotExist(JoinFilepaths(buildDirectory, "Shader"));
	CreateDirectoryIfItDoesNotExist(JoinFilepaths(buildDirectory, "Shader/GLSL"));
	CreateDirectoryIfItDoesNotExist(JoinFilepaths(buildDirectory, "Shader/GLSL/Code"));
	CreateDirectoryIfItDoesNotExist(JoinFilepaths(buildDirectory, "Shader/GLSL/Binary"));

	Append(&includeSearchDirectories, "Code");

	auto commonCompilerFlags = _FormatString("-std=c++17 -DUSE_VULKAN_RENDER_API -ICode -ffast-math -fno-exceptions -Wall -Wextra -Werror -Wfatal-errors -Wcast-align -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wredundant-decls -Wshadow -Wstrict-overflow=2 -Wundef -Wno-unused -Wno-sign-compare -Wno-missing-field-initializers");
	if (debugBuild)
	{
		Append(&commonCompilerFlags, " -DDEBUG -g -O0");
	}
	else
	{
		Append(&commonCompilerFlags, " -O3");
	}
	if (developmentBuild)
	{
		Append(&commonCompilerFlags, " -DDEVELOPMENT");
	}
	auto gameCompilerFlags = Concatenate(commonCompilerFlags, " -IDependencies/Vulkan/1.1.106.0/include");
	auto gameLinkerFlags = "Build/libBasic.so Build/libEngine.so Build/libMedia.so -lX11 -ldl -lm -lfreetype -lXi -lassimp -lpthread";

	String buildTarget;
	bool forceBuildFlag = false;
	for (auto i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			if (CStringsEqual(argv[i], "-f"))
			{
				forceBuildFlag = true;
			}
		}
		else
		{
			if (Length(buildTarget) != 0)
			{
				Abort("multiple build targets: %s and %s", argv[i], &buildTarget[0]);
			}
			buildTarget = argv[i];
		}
	}
	if (Length(buildTarget) == 0)
	{
		buildTarget = "Game";
	}

	Array<String> buildNames;
	if (buildTarget == "Game")
	{
		Append(&buildNames, String{"Basic"});
		Append(&buildNames, String{"Media"});
		Append(&buildNames, String{"Engine"});
		Append(&buildNames, String{"Game"});
	}
	else if (buildTarget == "Media")
	{
		Append(&buildNames, String{"Basic"});
		Append(&buildNames, String{"Media"});
	}
	else if (buildTarget == "Basic")
	{
		Append(&buildNames, String{"Basic"});
	}
	else if (buildTarget == "ShaderCompiler")
	{
		Append(&buildNames, String{"Basic"});
		Append(&buildNames, String{"ShaderCompiler"});
	}

	Array<BuildCommand> buildCommands;
	for (auto &name : buildNames)
	{
		if (name == "Basic")
		{
			Append(&buildCommands,
			{
				.sourceFilepath = "Code/Basic/Basic.cpp",
				.compilerFlags = commonCompilerFlags,
				.linkerFlags = "-fPIC",
				.binaryFilepath = "Build/libBasic.so",
				.isLibrary = true,
			});
		}
		else if (name == "Media")
		{
			Append(&buildCommands,
			{
				.sourceFilepath = "Code/Media/Media.cpp",
				.compilerFlags = commonCompilerFlags,
				.linkerFlags = "-fPIC",
				.binaryFilepath = "Build/libMedia.so",
				.isLibrary = true,
			});
		}
		else if (name == "Engine")
		{
			Append(&buildCommands,
			{
				.sourceFilepath = "Code/Engine/Engine.cpp",
				.compilerFlags = commonCompilerFlags,
				.linkerFlags = "-fPIC",
				.binaryFilepath = "Build/libEngine.so",
				.isLibrary = true,
			});
		}
		else if (name == "Game")
		{
			Append(&buildCommands,
			{
				.sourceFilepath = "Code/Game/Game.cpp",
				.compilerFlags = gameCompilerFlags,
				.linkerFlags = gameLinkerFlags,
				.binaryFilepath = "Build/Game",
				.isLibrary = false,
			});
		}
		else if (name == "ShaderCompiler")
		{
			Append(&buildCommands,
			{
				.sourceFilepath = "Code/ShaderCompiler/ShaderCompiler.cpp",
				.compilerFlags = commonCompilerFlags,
				.linkerFlags = "Build/libBasic.so",
				.binaryFilepath = "Build/ShaderCompiler",
				.isLibrary = false,
			});
		}
		else
		{
			Abort("unknown build command name: %s", &name[0]);
		}
	}

	LogPrint(LogType::INFO, "-------------------------------------------------------------------------------------------------\n");
	for (auto &command : buildCommands)
	{
		auto success = false;
		if (forceBuildFlag)
		{
			success = Build(command);
		}
		else
		{
			success = BuildIfOutOfDate(command);
		}
		if (!success)
		{
			LogPrint(LogType::ERROR, "\nbuild failed\n");
			LogPrint(LogType::INFO, "-------------------------------------------------------------------------------------------------\n");
			return 1;
		}
		LogPrint(LogType::INFO, "-------------------------------------------------------------------------------------------------\n");
	}

	return 0;
}
