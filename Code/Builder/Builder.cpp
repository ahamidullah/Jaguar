#include "Basic/Basic.h"

void RecursivelyGatherSourceFiles(const String &directory, Array<String> *files)
{
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
}

void BuildIfOutOfDate(const String &sourceFilepath, bool isLibrary, const String &compilerFlags, const String &linkerFlags, const String &binaryFilepath)
{
	auto directory = GetFilepathDirectory(sourceFilepath);
	Assert(Length(directory) > 0);

	Array<String> dependencies;
	Append(&dependencies, sourceFilepath);
	RecursivelyGatherSourceFiles(directory, &dependencies);

	bool needsBuild = false;

	if (FileExists(binaryFilepath))
	{
		auto [binaryFile, binaryOpenError] = OpenFile(binaryFilepath, OPEN_FILE_READ_ONLY);
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
				needsBuild = true;
				break;
			}
		}
	}
	else
	{
		needsBuild = true;
	}

	if (needsBuild)
	{
		LogPrint(LogType::INFO, "building: sourceFilepath: %s, isLibrary: %d, compilerFlags: %s, linkerFlags: %s, binaryFilepath: %s\n", &sourceFilepath[0], isLibrary, &compilerFlags[0], &linkerFlags[0], &binaryFilepath[0]);

		auto objFilepath = CreateString(binaryFilepath);
		if (!SetFilepathExtension(&objFilepath, "o"))
		{
			LogPrint(LogType::ERROR, "failed to create object filepath\n");
			return;
		}

		if (isLibrary)
		{
			auto compileCommand = _FormatString("g++ -c %s %s %s -o %s", &compilerFlags[0], &sourceFilepath[0], &linkerFlags[0], &objFilepath[0]);
			auto makeLibraryCommand = _FormatString("gcc -shared -o %s %s", &binaryFilepath[0], &objFilepath[0]);
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
			auto compileCommand = _FormatString("g++ %s %s %s -o %s", compilerFlags, &sourceFilepath[0], &linkerFlags[0], &binaryFilepath[0]);
			if (auto result = RunProcess(compileCommand); result != 0)
			{
				LogPrint(LogType::ERROR, "failed running compile command\n");
				return;
			}
		}

		LogPrint(LogType::INFO, "build complete\n");
	}
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

	auto commonCompilerFlags = _FormatString(" -std=c++17 -I%s -ffast-math -fno-exceptions -Wall -Wextra -Werror -Wfatal-errors -Wcast-align -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wredundant-decls -Wshadow -Wstrict-overflow=2 -Wundef -Wno-unused -Wno-sign-compare -Wno-missing-field-initializers", &codeDirectory[0]);
	auto gameCompilerFlags = Concatenate(commonCompilerFlags, " -IDependencies/Vulkan/1.1.106.0/include");
	auto gameLinkerFlags = _FormatString("%s/libMedia.a %s/libBasic.a -lX11 -ldl -lm -lfreetype -lXi -lassimp -lpthread", &buildDirectory[0], &buildDirectory[0]);

	if (debug)
	{
		Append(&commonCompilerFlags, " -DDEBUG -g -O0");
	}
	else
	{
		Append(&commonCompilerFlags, " -O3");
	}

	BuildIfOutOfDate(JoinFilepaths(codeDirectory, "Basic/Basic.cpp"), true, commonCompilerFlags, " -fPIC", JoinFilepaths(buildDirectory, "libBasic.so"));
	BuildIfOutOfDate(JoinFilepaths(codeDirectory, "Media/Media.cpp"), true, commonCompilerFlags, " -fPIC", JoinFilepaths(buildDirectory, "libMedia.so"));
	BuildIfOutOfDate(JoinFilepaths(codeDirectory, "Engine/Engine.cpp"), false, gameCompilerFlags, gameLinkerFlags, JoinFilepaths(buildDirectory, "Engine"));
}
