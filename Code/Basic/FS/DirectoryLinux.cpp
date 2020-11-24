#include "DirectoryLinux.h"

namespace filesystem
{

bool DirectoryIteration::Iterate(string::String path)
{
	if (!this->dir)
	{
		this->dir = opendir(path.ToCString());
		if (!this->dir)
		{
			log::Error("File", "Failed to open directory %k: %k.", path, PlatformError());
			return false;
		}
	}
	while ((this->dirent = readdir(this->dir)))
	{
		if (string::Equal(this->dirent->d_name, ".") || string::Equal(this->dirent->d_name, ".."))
		{
			continue;
		}
		this->filename = this->dirent->d_name;
		this->isDirectory = (this->dirent->d_type == DT_DIR);
		return true;
	}
	return false;
}

bool CreateDirectory(string::String path)
{
	if (mkdir(path.ToCString(), 0700) == -1)
	{
		log::Error("File", "Failed to create directory %k: %k.", path, PlatformError());
		return false;
	}
	return true;
}

bool CreateDirectoryIfItDoesNotExist(string::String path)
{
	if (Exists(path))
	{
		return true;
	}
	return CreateDirectory(path);
}

}
