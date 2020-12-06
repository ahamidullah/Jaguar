#include "Directory_Linux.h"

namespace fs
{

bool DirectoryIteration::Iterate(str::String path)
{
	if (!this->dir)
	{
		this->dir = opendir(path.CString());
		if (!this->dir)
		{
			log::Error("File", "Failed to open directory %k: %k.", path, PlatformError());
			return false;
		}
	}
	while ((this->dirent = readdir(this->dir)))
	{
		if (str::Equal(this->dirent->d_name, ".") || str::Equal(this->dirent->d_name, ".."))
		{
			continue;
		}
		this->filename = this->dirent->d_name;
		this->isDirectory = (this->dirent->d_type == DT_DIR);
		return true;
	}
	return false;
}

bool CreateDirectory(str::String path)
{
	if (mkdir(path.CString(), 0700) == -1)
	{
		log::Error("File", "Failed to create directory %k: %k.", path, PlatformError());
		return false;
	}
	return true;
}

bool CreateDirectoryIfItDoesNotExist(str::String path)
{
	if (Exists(path))
	{
		return true;
	}
	return CreateDirectory(path);
}

}
