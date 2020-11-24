#pragma once

namespace filesystem
{

struct DirectoryIteration
{
	DIR *dir;
	struct dirent *dirent;
	string::String filename;
	bool isDirectory;

	bool Iterate(string::String path);
};

bool CreateDirectory(string::String path);
bool CreateDirectoryIfItDoesNotExist(string::String path);

}
