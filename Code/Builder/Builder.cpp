#include "Basic/Basic.h"

bool debugBuild = true;
bool developmentBuild = true;

Array<String> includeSearchDirectories;
Array<String> librariesWithChangedHeaders;

bool GatherProjectFiles(const String &sourceFilepath, Array<String> *dependencies)
{
	DirectoryIteration iteration;
	while (IterateDirectory(sourceFilepath, &iteration))
	{
		if (iteration.isDirectory)
		{
			GatherProjectFiles(JoinFilepaths(sourceFilepath, iteration.filename), dependencies);
		}
		auto ext = GetFilepathExtension(iteration.filename);
		if (ext == ".cpp" || ext == ".h")
		{
			Append(dependencies, JoinFilepaths(sourceFilepath, iteration.filename));
		}
	}
	return true;
}

struct BuildCommand
{
	String name;
	String sourceFilepath;
	String compilerFlags;
	String linkerFlags;
	String binaryFilepath;
	Array<String> libraryDependencies;
	bool isLibrary;
};

bool Build(const BuildCommand &command)
{
	LogPrint
	(
		INFO_LOG,
		"sourceFilepath: %s\n\n"
		"isLibrary: %d\n\n"
		"compilerFlags: %s\n\n"
		"linkerFlags: %s\n\n"
		"binaryFilepath: %s\n\n",
		&command.sourceFilepath[0], command.isLibrary, &command.compilerFlags[0], &command.linkerFlags[0], &command.binaryFilepath[0]
	);

	auto objFilepath = CreateStringCopy(command.binaryFilepath);
	SetFilepathExtension(&objFilepath, ".o");

	if (command.isLibrary)
	{
		auto compileCommand = FormatString("g++ -shared %s %s %s -o %s", &command.compilerFlags[0], &command.sourceFilepath[0], &command.linkerFlags[0], &command.binaryFilepath[0]);
		LogPrint(LogType::INFO, "compileCommand: %s\n\n", &compileCommand[0]);
		if (auto result = RunProcess(compileCommand); result != 0)
		{
			return false;
		}
	}
	else
	{
		auto compileCommand = FormatString("g++ %s %s %s -o %s", &command.compilerFlags[0], &command.sourceFilepath[0], &command.linkerFlags[0], &command.binaryFilepath[0]);
		LogPrint(LogType::INFO, "compileCommand: %s\n\n", &compileCommand[0]);
		if (auto result = RunProcess(compileCommand); result != 0)
		{
			return false;
		}
	}

	LogPrint(LogType::INFO, "build successful\n");

	return true;
}

bool BuildIfOutOfDate(const BuildCommand &command)
{
	auto directory = GetFilepathDirectory(command.sourceFilepath);
	Assert(Length(directory) > 0);

	bool binaryExists = FileExists(command.binaryFilepath);

	bool libHeaderChanged = false;
	if (!binaryExists)
	{
		libHeaderChanged = true;
	}
	else
	{
		for (const auto &dep : command.libraryDependencies)
		{
			for (const auto &lib : librariesWithChangedHeaders)
			{
				if (dep == lib)
				{
					libHeaderChanged = true;
					break;
				}
			}
			if (libHeaderChanged)
			{
				break;
			}
		}
	}

	bool projectFileChanged = false;
	if (binaryExists && (command.isLibrary || !libHeaderChanged))
	{
		Array<String> projectFiles;
		if (!GatherProjectFiles(GetFilepathDirectory(command.sourceFilepath), &projectFiles))
		{
			return false;
		}

		auto [binaryFile, binaryOpenError] = OpenFile(command.binaryFilepath, OPEN_FILE_READ_ONLY);
		Assert(!binaryOpenError);
		auto binaryLastModifiedTime = GetFileLastModifiedTime(binaryFile);

		for (auto &sourceFilename : projectFiles)
		{
			auto [sourceFile, sourceOpenError] = OpenFile(sourceFilename, OPEN_FILE_READ_ONLY);
			Assert(!sourceOpenError);
			Defer(CloseFile(sourceFile));
			auto lastModifiedTime = GetFileLastModifiedTime(sourceFile);
			if (lastModifiedTime > binaryLastModifiedTime)
			{
				projectFileChanged = true;
				if (command.isLibrary)
				{
					auto ext = GetFilepathExtension(sourceFilename);
					if (ext == ".h")
					{
						auto path = GetFilepathFilename(command.sourceFilepath);
						SetFilepathExtension(&path, "");
						Append(&librariesWithChangedHeaders, path);
						break;
					}
				}
				else
				{
					break;
				}
			}
		}
	}

	if (libHeaderChanged || projectFileChanged || !binaryExists)
	{
		if (!binaryExists)
		{
			LogPrint(LogType::INFO, "%s does not exist, building...\n\n", &command.binaryFilepath[0]);
		}
		else if (libHeaderChanged || projectFileChanged)
		{
			LogPrint(LogType::INFO, "%s is out of date, rebuilding...\n\n", &command.binaryFilepath[0]);
		}
		return Build(command);
	}
	else
	{
		LogPrint(LogType::INFO, "%s is up to date, skipping build\n", &command.binaryFilepath[0]);
	}

	return true;
}

s32 ApplicationEntry(s32 argc, char *argv[])
{
	CreateDirectoryIfItDoesNotExist("Build/Shader");
	CreateDirectoryIfItDoesNotExist("Build/Shader/Code");
	CreateDirectoryIfItDoesNotExist("Build/Shader/Binary");

	Append(&includeSearchDirectories, String{"Code"});

	auto commonCompilerFlags = FormatString("-std=c++17 -DUSE_VULKAN_RENDER_API -ICode -ffast-math -fno-exceptions -Wall -Wextra -Werror -Wfatal-errors -Wcast-align -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wredundant-decls -Wshadow -Wstrict-overflow=2 -Wundef -Wno-unused -Wno-sign-compare -Wno-missing-field-initializers");
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
		else if (CStringsEqual(argv[i], "ShaderCompiler"))
		{
			buildTarget = "ShaderCompiler";
		}
		else
		{
			// @TODO
			Assert(0);
		}
	}

	auto allBuildCommands =
	CreateInitializedArray(
		BuildCommand{
			.name = "Basic",
			.sourceFilepath = "Code/Basic/Basic.cpp",
			.compilerFlags = commonCompilerFlags,
			.linkerFlags = "-fPIC",
			.binaryFilepath = "Build/libBasic.so",
			.isLibrary = true,
		},
		BuildCommand{
			.name = "Media",
			.sourceFilepath = "Code/Media/Media.cpp",
			.compilerFlags = commonCompilerFlags,
			.linkerFlags = "-fPIC",
			.binaryFilepath = "Build/libMedia.so",
			.libraryDependencies = CreateInitializedArray(String{"Basic"}),
			.isLibrary = true,
		},
		BuildCommand{
			.name = "Engine",
			.sourceFilepath = "Code/Engine/Engine.cpp",
			.compilerFlags = commonCompilerFlags,
			.linkerFlags = "-fPIC",
			.binaryFilepath = "Build/libEngine.so",
			.libraryDependencies = CreateInitializedArray(String{"Basic"}, String{"Media"}),
			.isLibrary = true,
		},
		BuildCommand{
			.name = "ShaderCompiler",
			.sourceFilepath = "Code/ShaderCompiler/ShaderCompiler.cpp",
			.compilerFlags = commonCompilerFlags,
			.linkerFlags = "Build/libBasic.so -ldl -lpthread",
			.binaryFilepath = "Build/ShaderCompiler",
			.libraryDependencies = CreateInitializedArray(String{"Basic"}),
			.isLibrary = false,
		},
		BuildCommand{
			.name = "Game",
			.sourceFilepath = "Code/Game/Game.cpp",
			.compilerFlags = gameCompilerFlags,
			.linkerFlags = gameLinkerFlags,
			.binaryFilepath = "Build/Game",
			.libraryDependencies = CreateInitializedArray(String{"Basic"}, String{"Media"}, String{"Engine"}),
			.isLibrary = false,
		}
	);

	Array<BuildCommand> requestedBuildCommands;
	if (buildTarget == "")
	{
		requestedBuildCommands = allBuildCommands;
	}
	else
	{
		auto GetBuildCommand = [&allBuildCommands](const String &name)
		{
			for (auto command : allBuildCommands)
			{
				if (name == command.name)
				{
					return command;
				}
			}
			Abort("unknown build name %s\n", &name[0]);
			return BuildCommand{};
		};
		auto command = GetBuildCommand(buildTarget);
		for (auto dep : command.libraryDependencies)
		{
			Append(&requestedBuildCommands, GetBuildCommand(dep));
		}
		Append(&requestedBuildCommands, command);
	}

	LogPrint(LogType::INFO, "-------------------------------------------------------------------------------------------------\n");
	for (auto &command : requestedBuildCommands)
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
