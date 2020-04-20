#include "Basic/Basic.h"

bool debugBuild = true;
bool developmentBuild = true;

struct BuildCommand
{
	String        name;
	String        sourceFilepath;
	String        compilerFlags;
	String        linkerFlags;
	String        binaryFilepath;
	Array<String> dependencies;
	bool          isLibrary;
};

bool Build(const BuildCommand &command)
{
	LogPrint(INFO_LOG,
	         "Running build command %k...\n\n"
	         "sourceFilepath: %k\n\n"
	         "isLibrary: %d\n\n"
	         "compilerFlags: %k\n\n"
	         "linkerFlags: %k\n\n"
	         "binaryFilepath: %k\n\n",
	         command.name,
	         command.sourceFilepath,
	         command.isLibrary,
	         command.compilerFlags,
	         command.linkerFlags,
	         command.binaryFilepath);

	auto objFilepath = CreateString(command.binaryFilepath);
	SetFilepathExtension(&objFilepath, ".o");

	if (command.isLibrary)
	{
		auto compileCommand = FormatString("g++ -shared %k %k %k -o %k", command.compilerFlags, command.sourceFilepath, command.linkerFlags, command.binaryFilepath);
		LogPrint(INFO_LOG, "compileCommand: %k\n\n", compileCommand);
		if (RunProcess(compileCommand) != 0)
		{
			LogPrint(INFO_LOG, "\nBuild failed.\n");
			return false;
		}
	}
	else
	{
		auto compileCommand = FormatString("g++ %k %k %k -o %k", command.compilerFlags, command.sourceFilepath, command.linkerFlags, command.binaryFilepath);
		LogPrint(INFO_LOG, "compileCommand: %k\n\n", compileCommand);
		if (auto result = RunProcess(compileCommand); result != 0)
		{
			LogPrint(INFO_LOG, "\nBuild failed.\n");
			return false;
		}
	}

	LogPrint(INFO_LOG, "Build successful.\n");
	return true;
}

Array<String> GatherModuleFilepaths(const String &sourceDirectory)
{
	Array<String> directories;
	ArrayAppend(&directories, sourceDirectory);

	Array<String> result;
	for (auto i = 0; i < ArrayLength(directories); i++)
	{
		DirectoryIteration iteration;
		while (IterateDirectory(directories[i], &iteration))
		{
			auto filepath = JoinFilepaths(directories[i], iteration.filename);
			if (iteration.isDirectory)
			{
				ArrayAppend(&directories, filepath);
				continue;
			}
			auto ext = GetFilepathExtension(iteration.filename);
			if (ext == ".cpp" || ext == ".h")
			{
				ArrayAppend(&result, filepath);
			}
		}
	}
	return result;
}

bool BuildIfOutOfDate(const BuildCommand &command)
{
	struct LibraryBuildInfo
	{
		String buildCommandName;
		bool   dependentsNeedRebuild;
	};
	static Array<LibraryBuildInfo> libraryBuildInfos;

	if (!FileExists(command.binaryFilepath))
	{
		LogPrint(INFO_LOG, "%k does not exist, building...\n\n", command.binaryFilepath);
		if (command.isLibrary)
		{
			ArrayAppend(&libraryBuildInfos,
		            	{
		                	.buildCommandName = command.name, // @TODO: Take a copy?
		           	    	.dependentsNeedRebuild = true,
		            	});
		}
		return Build(command);
	}

	auto dependencyFileWasModified = false;
	for (const auto &d : command.dependencies)
	{
		auto foundDependencyLibraryBuildInfo = false;
		for (const auto &i : libraryBuildInfos)
		{
			if (i.buildCommandName == d)
			{
				foundDependencyLibraryBuildInfo = true;
				if (i.dependentsNeedRebuild)
				{
					dependencyFileWasModified = true;
					break;
				}
			}
		}
		if (!foundDependencyLibraryBuildInfo)
		{
			LogPrint(ERROR_LOG, "ERROR: In build command %k: could not find library build info for dependency %k.\n", command.name, d);
			return false;
		}
	}

	auto moduleFileWasModified = false;
	auto libraryHeaderChanged = false;
	if (command.isLibrary || !dependencyFileWasModified)
	{
		bool error = false;

		auto sourceDirectory = GetFilepathDirectory(command.sourceFilepath);
		Assert(StringLength(sourceDirectory) > 0);

		auto moduleFilepaths = GatherModuleFilepaths(sourceDirectory);

		auto binaryModifiedTime = GetFilepathLastModifiedTime(command.binaryFilepath, &error);
		if (error)
		{
			LogPrint(ERROR_LOG, "ERROR: In build command %k: could not find last modified time for binary file %k.\n", command.name, command.binaryFilepath);
			return false;
		}

		for (auto &filepath : moduleFilepaths)
		{
			auto sourceModifiedTime = GetFilepathLastModifiedTime(filepath, &error);
			if (error)
			{
				Abort("Could not find last modified time for source file %k, retrieved for build command %k", filepath, command.name);
			}
			if (sourceModifiedTime > binaryModifiedTime)
			{
				moduleFileWasModified = true;
				if (command.isLibrary)
				{
					auto ext = GetFilepathExtension(filepath);
					if (ext == ".h")
					{
						libraryHeaderChanged = true;
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

	if (command.isLibrary)
	{
		if (libraryHeaderChanged || dependencyFileWasModified)
		{
			ArrayAppend(&libraryBuildInfos,
						{
							.buildCommandName = command.name, // @TODO: Take a copy?
							.dependentsNeedRebuild = true,
						});
		}
		else
		{
			ArrayAppend(&libraryBuildInfos,
						{
							.buildCommandName = command.name, // @TODO: Take a copy?
							.dependentsNeedRebuild = false,
						});
		}
	}

	if (moduleFileWasModified || dependencyFileWasModified)
	{
		LogPrint(INFO_LOG, "%k is out of date, rebuilding...\n\n", command.binaryFilepath);
		return Build(command);
	}

	LogPrint(INFO_LOG, "%k is up to date, skipping build.\n", command.binaryFilepath);

	return true;
}

s32 ApplicationEntry(s32 argc, char *argv[])
{
	CreateDirectoryIfItDoesNotExist("Build/Shader");
	CreateDirectoryIfItDoesNotExist("Build/Shader/Code");
	CreateDirectoryIfItDoesNotExist("Build/Shader/Binary");

	bool error = false;

	// @TODO: Some way to set render api compiler flag (-DUSE_VULKAN_RENDER_API).
	auto commonCompilerFlags = ReadEntireFile("Code/Builder/CommonCompilerFlags.txt", &error);
	if (error)
	{
		Abort("Could not read file Code/Builder/CommonCompilerFlags.txt.");
	}
	if (StringLength(commonCompilerFlags) > 0 && commonCompilerFlags[StringLength(commonCompilerFlags) - 1] == '\n')
	{
		ResizeString(&commonCompilerFlags, StringLength(commonCompilerFlags) - 1);
	}
	if (debugBuild)
	{
		StringAppend(&commonCompilerFlags, " -DDEBUG -g -O0");
	}
	else
	{
		StringAppend(&commonCompilerFlags, " -O3");
	}
	if (developmentBuild)
	{
		StringAppend(&commonCompilerFlags, " -DDEVELOPMENT");
	}
	auto gameCompilerFlags = JoinStrings(commonCompilerFlags, " -IDependencies/Vulkan/1.1.106.0/include");
	auto gameLinkerFlags = "Build/libBasic.so Build/libEngine.so Build/libMedia.so -lX11 -ldl -lm -lfreetype -lXi -lassimp -lpthread";

	bool forceBuildFlag = false;
	for (auto i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			if (CStringsEqual(argv[i], "-f"))
			{
				forceBuildFlag = true;
			}
			else
			{
				LogPrint(ERROR_LOG, "Unknown flag: %s.\n", argv[i]);
				return PROCESS_FAILURE;
			}
		}
	}

	auto allBuildCommands = CreateArray<BuildCommand>(0);
	{
		BuildCommand buildCommand =
		{
			.name = "Basic",
			.sourceFilepath = "Code/Basic/Basic.cpp",
			.compilerFlags = commonCompilerFlags,
			.linkerFlags = "-fPIC",
			.binaryFilepath = "Build/libBasic.so",
			.isLibrary = true,
		};
		ArrayAppend(&allBuildCommands, buildCommand);
	}
	{
		BuildCommand buildCommand =
		{
			.name = "Media",
			.sourceFilepath = "Code/Media/Media.cpp",
			.compilerFlags = commonCompilerFlags,
			.linkerFlags = "-fPIC",
			.binaryFilepath = "Build/libMedia.so",
			.isLibrary = true,
		};
		ArrayAppend(&buildCommand.dependencies, String{"Basic"});
		ArrayAppend(&allBuildCommands, buildCommand);
	}
	{
		BuildCommand buildCommand =
		{
			.name = "Engine",
			.sourceFilepath = "Code/Engine/Engine.cpp",
			.compilerFlags = commonCompilerFlags,
			.linkerFlags = "-fPIC",
			.binaryFilepath = "Build/libEngine.so",
			.isLibrary = true,
		};
		ArrayAppend(&buildCommand.dependencies, String{"Media"});
		ArrayAppend(&allBuildCommands, buildCommand);
	}
	{
		BuildCommand buildCommand =
		{
			.name = "Game",
			.sourceFilepath = "Code/Game/Game.cpp",
			.compilerFlags = gameCompilerFlags,
			.linkerFlags = gameLinkerFlags,
			.binaryFilepath = "Build/Game",
			.isLibrary = false,
		};
		ArrayAppend(&buildCommand.dependencies, String{"Engine"});
		ArrayAppend(&allBuildCommands, buildCommand);
	}
	/*
	{
		BuildCommand buildCommand =
		{
			.name = "ShaderCompiler",
			.sourceFilepath = "Code/ShaderCompiler/ShaderCompiler.cpp",
			.compilerFlags = commonCompilerFlags,
			.linkerFlags = "Build/libBasic.so -ldl -lpthread",
			.binaryFilepath = "Build/ShaderCompiler",
			.isLibrary = false,
		};
		ArrayAppend(&buildCommand.dependencies, String{"Basic"});
		ArrayAppend(&allBuildCommands, buildCommand);
	}
	*/

	LogPrint(INFO_LOG, "-------------------------------------------------------------------------------------------------\n");
	for (auto &command : allBuildCommands)
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
		LogPrint(INFO_LOG, "-------------------------------------------------------------------------------------------------\n");
		if (!success)
		{
			return PROCESS_FAILURE;
		}
	}

	return PROCESS_SUCCESS;
}
