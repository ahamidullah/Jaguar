#include "Basic/Basic.h"

struct Flags
{
	bool force = false;
	bool verbose = false;
	bool debug = true;
	bool development = true;
} flags;

enum BuildType
{
	EXECUTABLE_BUILD,
	LIBRARY_BUILD,
};

struct BuildCommand
{
	String name;
	String sourceFilepath;
	String compilerFlags;
	String linkerFlags;
	String binaryFilepath;
	String pchHeaderFilepath;
	Array<String> directModuleDependencies;
	BuildType buildType;

	Array<BuildCommand *> dependencies;
	String pchFilepath;
};

struct BuildRecord
{
	bool dependentsNeedRebuild;
};
auto buildRecords = HashTable<String, BuildRecord>{};

bool BuildPCH(const BuildCommand &command)
{
	auto compileCommand = FormatString("clang++ -x -cpp-header %k %k -o %k", command.compilerFlags, command.pchHeaderFilepath, command.pchFilepath);
	if (RunProcess(compileCommand) != 0)
	{
		return false;
	}
	return true;
}

bool BuildBinary(const BuildCommand &command)
{
	auto pchIncludes = String{};
	for (auto &dep : command.dependencies)
	{
		if (dep->pchFilepath != "")
		{
			StringAppend(&pchIncludes, FormatString(" %k", dep->pchFilepath));
		}
	}

	auto compileCommand = String{"clang++"};
	if (pchIncludes != "")
	{
		StringAppend(&compileCommand, " -include-pch");
		StringAppend(&compileCommand, pchIncludes);
	}
	if (command.buildType == EXECUTABLE_BUILD)
	{
		StringAppend(&compileCommand, FormatString(" %k %k %k -o %k", command.compilerFlags, command.sourceFilepath, command.linkerFlags, command.binaryFilepath));
	}
	else
	{
		StringAppend(&compileCommand, FormatString(" -shared %k %k %k -o %k", command.compilerFlags, command.sourceFilepath, command.linkerFlags, command.binaryFilepath));
	}
	if (RunProcess(compileCommand) != 0)
	{
		return false;
	}
	return true;
}

bool IsPCHOutOfDate(const BuildCommand &command)
{
	auto error = false;
	auto pchModifiedTime = GetFilepathLastModifiedTime(command.pchFilepath, &error);
	if (error)
	{
		Abort("Could not find last modified time for PCH file %k, in build command %k.\n", command.pchFilepath, command.name);
	}
	auto pchHeaderModifiedTime = GetFilepathLastModifiedTime(command.pchHeaderFilepath, &error);
	if (error)
	{
		Abort("Could not find last modified time for PCH header file %k, in build command %k.\n", command.pchHeaderFilepath, command.name);
	}
	return pchHeaderModifiedTime > pchModifiedTime;
}

Array<String> GatherModuleFilepaths(const String &sourceDirectory)
{
	auto directories = Array<String>{};
	ArrayAppend(&directories, sourceDirectory);

	auto result = Array<String>{};
	for (auto i = 0; i < directories.count; i++)
	{
		auto iteration = DirectoryIteration{};
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

bool IsBinaryOutOfDate(const BuildCommand &command, bool *dependentsNeedRebuild)
{
	for (auto &dep : command.directModuleDependencies)
	{
		auto depBuildRecord = LookupInHashTable(&buildRecords, dep);
		if (!depBuildRecord)
		{
			Abort("Could not find library build record for dependency %k, in build command %k.\n", dep, command.name);
		}
		if (depBuildRecord->dependentsNeedRebuild)
		{
			*dependentsNeedRebuild = true;
			return true;
		}
	}

	auto error = false;

	auto binaryModifiedTime = GetFilepathLastModifiedTime(command.binaryFilepath, &error);
	if (error)
	{
		Abort("Could not find last modified time for binary file %k, in build command %k.\n", command.binaryFilepath, command.name);
	}

	auto moduleChanged = false;
	auto libraryHeaderChanged = false;
	auto sourceDirectory = GetFilepathDirectory(command.sourceFilepath);
	Assert(StringLength(sourceDirectory) > 0);
	auto moduleFilepaths = GatherModuleFilepaths(sourceDirectory);
	for (auto &filepath : moduleFilepaths)
	{
		auto sourceModifiedTime = GetFilepathLastModifiedTime(filepath, &error);
		if (error)
		{
			Abort("Could not find last modified time for source file %k, in build command %k.\n", filepath, command.name);
		}
		if (sourceModifiedTime > binaryModifiedTime)
		{
			moduleChanged = true;
			if (command.buildType == LIBRARY_BUILD)
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

	if (libraryHeaderChanged || (moduleChanged && command.buildType == EXECUTABLE_BUILD))
	{
		*dependentsNeedRebuild = true;
	}

	return moduleChanged;
}

#if 0
bool Build(const BuildCommand &command)
{
	if (force)
	{
		return BuildExecutable();
	}

	auto record = BuildRecord{};
	if (IsBinaryOutOfDate(command, &record.dependentsNeedRebuild))
	{
		auto buildPCH = IsPCHOutOfDate(command);
		if (!BuildSource(command, buildPCH))
		{
			return false;
		}
	}
	InsertIntoHashTable(&buildRecords, command.name, record);
	return true;
}

bool BuildIfOutOfDate(const BuildCommand &command)
{
	auto record = BuildRecord{};
	if (!FileExists(command.binaryFilepath) || (command.pchFilepath != "" && !FileExists(command.pchFilepath)))
	{
		record.dependentsNeedRebuild = true;
		InsertIntoHashTable(&buildRecords, command.name, record);
		return BuildSource();
	}
	if (IsBinaryOutOfDate(command, &record.dependentsNeedRebuild))
	{
		InsertIntoHashTable(&buildRecords, command.name, record);
		auto buildPCH = IsPCHOutOfDate(command);
		return BuildSource(command, buildPCH);
	}
	return true;
}

bool Build(const BuildCommand &command)
{
	LogPrint(INFO_LOG, "Running build command %k...\n\n", command.name);

	if (command.pchHeaderFilepath != "")
	{
		CreateDirectoryIfItDoesNotExist(StringAppend("Build/Linux/PCH/", command.name));

		auto pchFilepath = FormatString("Build/Linux/PCH/%k/PCH.h.pch", command.name);
		auto error = false;
		auto pchModifiedTime = GetFilepathLastModifiedTime(pchFilepath , &error);
		if (error)
		{
			LogPrint(ERROR_LOG, "Failed to get last modified time for file %k\n", pchFilepath);
			return false;
		}
		auto sourceModifiedTime = GetFilepathLastModifiedTime(command.precompiledHeaderSourceFilepath , &error);
		if (error)
		{
		}
	}

	auto compileCommand = String{};
	if (command.buildType == LIBRARY_BUILD)
	{
		else
		{
			compileCommand = FormatString("clang++ -shared %k %k %k -o %k", command.compilerFlags, command.sourceFilepath, command.linkerFlags, command.binaryFilepath);
		}
	}
	else if (command.buildType == EXECUTABLE_BUILD)
	{
		compileCommand = FormatString("clang++ %k %k %k -o %k", command.compilerFlags, command.sourceFilepath, command.linkerFlags, command.binaryFilepath);
	}
	if (RunProcess(compileCommand) != 0)
	{
		LogPrint(INFO_LOG, "\nBuild failed.\n");
		return false;
	}

	LogPrint(INFO_LOG, "Build successful.\n");
	return true;
}

bool BuildIfOutOfDate(const BuildCommand &command)
{
	struct LibraryBuildInfo
	{
		String buildCommandName;
		bool dependentsNeedRebuild;
	};
	static auto libraryBuildInfos = Array<LibraryBuildInfo>{};

	if (!FileExists(command.binaryFilepath))
	{
		LogPrint(INFO_LOG, "%k does not exist, building...\n\n", command.binaryFilepath);
		if (command.buildType == LIBRARY_BUILD)
		{
			ArrayAppend(
				&libraryBuildInfos,
				{
					.buildCommandName = command.name, // @TODO: Take a copy?
					.dependentsNeedRebuild = true,
				});
		}
		return Build(command);
	}

	auto dependencyFileWasModified = false;
	for (auto &d : command.dependencies)
	{
		auto foundDependencyLibraryBuildInfo = false;
		for (auto &i : libraryBuildInfos)
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
	if (command.buildType == LIBRARY_BUILD || !dependencyFileWasModified)
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
				if (command.buildType == LIBRARY_BUILD)
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

	if (command.buildType == LIBRARY_BUILD)
	{
		if (libraryHeaderChanged || dependencyFileWasModified)
		{
			ArrayAppend(
				&libraryBuildInfos,
				{
					.buildCommandName = command.name, // @TODO: Take a copy?
					.dependentsNeedRebuild = true,
				});
		}
		else
		{
			ArrayAppend(
				&libraryBuildInfos,
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
#endif

s32 ApplicationEntry(s32 argc, char *argv[])
{
	CreateDirectoryIfItDoesNotExist("Build/Shader");
	CreateDirectoryIfItDoesNotExist("Build/Shader/Code");
	CreateDirectoryIfItDoesNotExist("Build/Shader/Binary");
	CreateDirectoryIfItDoesNotExist("Build/Linux");
	CreateDirectoryIfItDoesNotExist("Build/Linux/Binary");
	CreateDirectoryIfItDoesNotExist("Build/Linux/PCH");

	auto error = false;

	// @TODO: Have these get passed in from the build script. Or at least the filepath becuase it should change based on platform.
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
	if (flags.debug)
	{
		StringAppend(&commonCompilerFlags, " -DDEBUG -g -O0");
	}
	else
	{
		StringAppend(&commonCompilerFlags, " -O3");
	}
	if (flags.development)
	{
		StringAppend(&commonCompilerFlags, " -DDEVELOPMENT");
	}
	auto gameCompilerFlags = JoinStrings(commonCompilerFlags, " -IDependencies/Vulkan/1.1.106.0/include");
	auto gameLinkerFlags = "Build/libBasic.so Build/libEngine.so Build/libMedia.so -lX11 -ldl -lm -lfreetype -lXi -lassimp -lpthread";

	for (auto i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			if (CStringsEqual(argv[i], "-f"))
			{
				flags.force = true;
			}
			else
			{
				LogPrint(ERROR_LOG, "Unknown flag: %s.\n", argv[i]);
				return PROCESS_EXIT_FAILURE;
			}
		}
	}

	auto allBuildCommands = CreateArray<BuildCommand>(0);
	{
		auto buildCommand = BuildCommand
		{
			.name = "Basic",
			.sourceFilepath = "Code/Basic/Basic.cpp",
			.compilerFlags = commonCompilerFlags,
			.linkerFlags = "-fPIC",
			.binaryFilepath = "Build/libBasic.so",
			.pchHeaderFilepath = "Code/Basic/PCH.h",
			.buildType = LIBRARY_BUILD,
		};
		ArrayAppend(&allBuildCommands, buildCommand);
	}
	{
		auto buildCommand = BuildCommand
		{
			.name = "Media",
			.sourceFilepath = "Code/Media/Media.cpp",
			.compilerFlags = commonCompilerFlags,
			.linkerFlags = "-fPIC",
			.binaryFilepath = "Build/libMedia.so",
			.buildType = LIBRARY_BUILD,
		};
		ArrayAppend(&buildCommand.directModuleDependencies, "Basic");
		ArrayAppend(&allBuildCommands, buildCommand);
	}
	{
		auto buildCommand = BuildCommand
		{
			.name = "Engine",
			.sourceFilepath = "Code/Engine/Engine.cpp",
			.compilerFlags = commonCompilerFlags,
			.linkerFlags = "-fPIC",
			.binaryFilepath = "Build/libEngine.so",
			.buildType = LIBRARY_BUILD,
		};
		ArrayAppend(&buildCommand.directModuleDependencies, "Media");
		ArrayAppend(&allBuildCommands, buildCommand);
	}
	{
		auto buildCommand = BuildCommand
		{
			.name = "Game",
			.sourceFilepath = "Code/Game/Game.cpp",
			.compilerFlags = gameCompilerFlags,
			.linkerFlags = gameLinkerFlags,
			.binaryFilepath = "Build/Game",
			.buildType = EXECUTABLE_BUILD,
		};
		ArrayAppend(&buildCommand.directModuleDependencies, "Engine");
		ArrayAppend(&allBuildCommands, buildCommand);
	}
	{
		auto buildCommand = BuildCommand
		{
			.name = "ShaderCompiler",
			.sourceFilepath = "Code/ShaderCompiler/ShaderCompiler.cpp",
			.compilerFlags = commonCompilerFlags,
			.linkerFlags = "Build/libBasic.so -ldl -lpthread",
			.binaryFilepath = "Build/ShaderCompiler",
			.buildType = EXECUTABLE_BUILD,
		};
		ArrayAppend(&buildCommand.directModuleDependencies, "Basic");
		ArrayAppend(&allBuildCommands, buildCommand);
	}

	for (auto &command : allBuildCommands)
	{
		for (auto &dep : command.directModuleDependencies)
		{
			for (auto i = 0; i < allBuildCommands.count; i++)
			{
				if (dep == allBuildCommands[i].name)
				{
					ArrayAppend(&command.dependencies, &allBuildCommands[i]);
				}
			}
		}

		if (command.pchHeaderFilepath != "")
		{
			command.pchFilepath = FormatString("Build/Linux/PCH/%k/PCH.h.pch", command.name);
		}
	}

	auto Fail = []()
	{
		LogPrint(INFO_LOG, "\nBuild failed.\n");
		LogPrint(INFO_LOG, "-------------------------------------------------------------------------------------------------\n");
		ExitProcess(PROCESS_EXIT_FAILURE);
	};

	LogPrint(INFO_LOG, "-------------------------------------------------------------------------------------------------\n");
	for (auto &command : allBuildCommands)
	{
		LogPrint(INFO_LOG, "Running build command %k...\n\n", command.name);

		auto record = BuildRecord{};
		if (flags.force)
		{
			record.dependentsNeedRebuild = true;
			if (!BuildPCH(command))
			{
				Fail();
			}
			if (!BuildBinary(command))
			{
				Fail();
			}
		}
		else
		{
			auto builtPCH = false;
			if (command.pchFilepath != "")
			{
				if (!FileExists(command.pchFilepath))
				{
					LogPrint(INFO_LOG, "Precompiled header for %k does not exist, building...\n\n", command.binaryFilepath);
					record.dependentsNeedRebuild = true;
					if (!BuildPCH(command))
					{
						Fail();
					}
					builtPCH = true;
				}
				else if (IsPCHOutOfDate(command))
				{
					LogPrint(INFO_LOG, "Precompiled header for %k is out of date, rebuilding...\n\n", command.binaryFilepath);
					record.dependentsNeedRebuild = true;
					if (!BuildPCH(command))
					{
						Fail();
					}
					builtPCH = true;
				}
			}

			if (!FileExists(command.binaryFilepath))
			{
				LogPrint(INFO_LOG, "%k does not exist, building...\n\n", command.binaryFilepath);
				record.dependentsNeedRebuild = true;
				if (!BuildBinary(command))
				{
					Fail();
				}
			}
			else if (builtPCH || IsBinaryOutOfDate(command, &record.dependentsNeedRebuild))
			{
				LogPrint(INFO_LOG, "%k is out of date, rebuilding...\n\n", command.binaryFilepath);
				if (!BuildBinary(command))
				{
					Fail();
				}
			}
			else
			{
				LogPrint(INFO_LOG, "%k is up to date, skipping build.\n", command.binaryFilepath);
			}
		}
		InsertIntoHashTable(&buildRecords, command.name, record);

		LogPrint(INFO_LOG, "Build successful.\n");
		LogPrint(INFO_LOG, "-------------------------------------------------------------------------------------------------\n");
	}

	return PROCESS_EXIT_SUCCESS;
}
