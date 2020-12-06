#pragma once

namespace fs
{

struct DirectoryIteration
{
	DIR *dir;
	struct dirent *dirent;
	str::String filename;
	bool isDirectory;

	bool Iterate(str::String path);
};

bool CreateDirectory(str::String path);
bool CreateDirectoryIfItDoesNotExist(str::String path);

}
