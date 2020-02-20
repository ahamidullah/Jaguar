#include "Basic/Basic.h"

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
			LogPrint(LogType::ERROR, "failed to gather dependencies for file %s\n", &queuedFiles[i][0]);
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
						LogPrint(LogType::ERROR, "failed to find file %s included from file %s\n", &includePartialFilepath[0], &queuedFiles[i][0]);
						return false;
					}
					Append(&queuedFiles, CleanFilepath(includeFilepath));
				}
			}
		}

		Append(&finishedFiles, queuedFiles[i]);
	}

	return true;
#if 0
	DirectoryIteration i;
	while (IterateDirectory(directory, &i))
	{
		auto filepath = JoinFilepaths(directory, i.filename);
		if (i.isDirectory)
		{
			RecursivelyGatherSourceFiles(filepath, files);
		}
		else
		{
			auto extension = GetFilepathExtension(i.filename);
			if (extension == "cpp" || extension == "h")
			{
				Append(files, filepath);
			}
		}
	}
#endif
}

struct BuildCommand
{
	String sourceFilepath;
	String compilerFlags;
	String linkerFlags;
	String binaryFilepath;
	bool isLibrary;
};

void Build(const BuildCommand &command)
{
	auto objFilepath = CreateString(command.binaryFilepath);
	SetFilepathExtension(&objFilepath, "o");

	if (command.isLibrary)
	{
		auto compileCommand = _FormatString("g++ -c %s %s %s -o %s", &command.compilerFlags[0], &command.sourceFilepath[0], &command.linkerFlags[0], &objFilepath[0]);
		auto makeLibraryCommand = _FormatString("gcc -shared -o %s %s", &command.binaryFilepath[0], &objFilepath[0]);
		LogPrint(LogType::INFO, "\tcompileCommand: %s\n\tmakeLibraryCommand: %s\n", &compileCommand[0], &makeLibraryCommand[0]);
		if (auto result = RunProcess(compileCommand); result != 0)
		{
			LogPrint(LogType::ERROR, "failed running compile command\n");
			return;
		}
		if (auto result = RunProcess(makeLibraryCommand); result != 0)
		{
			LogPrint(LogType::ERROR, "failed running make library command\n");
			return;
		}
	}
	else
	{
		auto compileCommand = _FormatString("g++ %s %s %s -o %s", &command.compilerFlags[0], &command.sourceFilepath[0], &command.linkerFlags[0], &command.binaryFilepath[0]);
		LogPrint(LogType::INFO, "\tcompileCommand: %s\n", &compileCommand[0]);
		if (auto result = RunProcess(compileCommand); result != 0)
		{
			LogPrint(LogType::ERROR, "failed running compile command\n");
			return;
		}
	}

	LogPrint(LogType::INFO, "build success\n");
}

void BuildIfOutOfDate(const BuildCommand &command)
{
	auto directory = GetFilepathDirectory(command.sourceFilepath);
	Assert(Length(directory) > 0);

	Array<String> dependencies;
	if (GatherDependencies(command.sourceFilepath, &dependencies))
	{
		LogPrint(LogType::INFO, "command:\n\tsourceFilepath: %s\n\tisLibrary: %d\n\tcompilerFlags: %s\n\tlinkerFlags: %s\n\tbinaryFilepath: %s\n\n", &command.sourceFilepath[0], command.isLibrary, &command.compilerFlags[0], &command.linkerFlags[0], &command.binaryFilepath[0]);

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
					LogPrint(LogType::INFO, "%s out of date, rebuilding...\n", &command.binaryFilepath[0]);
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
			LogPrint(LogType::INFO, "%s does not exist, building...\n", &command.binaryFilepath[0]);
			needsBuild = true;
		}

		if (needsBuild)
		{
			Build(command);
		}
	}

	LogPrint(LogType::INFO, "\n-------------------------------------------------------------------------\n\n");
}

s32 main(s32 argc, char *argv[])
{
	String projectDirectory = getenv("PROJECT_DIRECTORY");
	if (Length(projectDirectory) <= 0)
	{
		LogPrint(LogType::ERROR, "failed to get PROJECT_DIRECTORY enviornment variable\n");
		return 1;
	}
	auto codeDirectory = JoinFilepaths(projectDirectory, "Code");
	auto buildDirectory = JoinFilepaths(projectDirectory, "Build");
	Append(&includeSearchDirectories, codeDirectory);

	auto commonCompilerFlags = _FormatString(" -std=c++17 -DUSE_VULKAN_RENDER_API -I%s -ffast-math -fno-exceptions -Wall -Wextra -Werror -Wfatal-errors -Wcast-align -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wredundant-decls -Wshadow -Wstrict-overflow=2 -Wundef -Wno-unused -Wno-sign-compare -Wno-missing-field-initializers", &codeDirectory[0]);
	auto gameCompilerFlags = Concatenate(commonCompilerFlags, " -IDependencies/Vulkan/1.1.106.0/include");
	auto gameLinkerFlags = _FormatString("%s/libEngine.so %s/libMedia.so %s/libBasic.so -lX11 -ldl -lm -lfreetype -lXi -lassimp -lpthread", &buildDirectory[0], &buildDirectory[0], &buildDirectory[0]);
	auto builderLinkerFlags = _FormatString("%s/libBasic.so", &buildDirectory[0]);

	if (debug)
	{
		Append(&commonCompilerFlags, " -DDEBUG -g -O0");
	}
	else
	{
		Append(&commonCompilerFlags, " -O3");
	}

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

	Array<BuildCommand> buildCommands;
	for (auto &name : buildNames)
	{
		if (name == "Basic")
		{
			Append(&buildCommands, BuildCommand{
				.sourceFilepath = JoinFilepaths(codeDirectory, "Basic/Basic.cpp"),
				.compilerFlags = commonCompilerFlags,
				.linkerFlags = "-fPIC",
				.binaryFilepath = JoinFilepaths(buildDirectory, "libBasic.so"),
				.isLibrary = true,
			});
		}
		else if (name == "Media")
		{
			Append(&buildCommands, BuildCommand{
				.sourceFilepath = JoinFilepaths(codeDirectory, "Media/Media.cpp"),
				.compilerFlags = commonCompilerFlags,
				.linkerFlags = "-fPIC",
				.binaryFilepath = JoinFilepaths(buildDirectory, "libMedia.so"),
				.isLibrary = true,
			});
		}
		else if (name == "Engine")
		{
			Append(&buildCommands, BuildCommand{
				.sourceFilepath = JoinFilepaths(codeDirectory, "Engine/Engine.cpp"),
				.compilerFlags = commonCompilerFlags,
				.linkerFlags = "-fPIC",
				.binaryFilepath = JoinFilepaths(buildDirectory, "libEngine.so"),
				.isLibrary = true,
			});
		}
		else if (name == "Game")
		{
			Append(&buildCommands, BuildCommand{
				.sourceFilepath = JoinFilepaths(codeDirectory, "Game/Game.cpp"),
				.compilerFlags = gameCompilerFlags,
				.linkerFlags = gameLinkerFlags,
				.binaryFilepath = JoinFilepaths(buildDirectory, "Game"),
				.isLibrary = false,
			});
		}
		else
		{
			Abort("unknown build command name: %s", &name[0]);
		}
	}

	if (forceBuildFlag)
	{
		for (auto &command : buildCommands)
		{
			Build(command);
		}
	}
	else
	{
		for (auto &command : buildCommands)
		{
			BuildIfOutOfDate(command);
		}
	}
}
