#include "../File.h"
#include "../Log.h"

File OpenFile(String path, s64 flags, bool *err)
{
	auto f = File
	{
		.path = NewStringCopy(path),
	};
	f.handle = open(&path[0], flags, 0666);
	if (f.handle < 0)
	{
		LogPrint(LogLevelError, "File", "Failed to open file %k: %k.\n", path, GetPlatformError());
		*err = true;
		return {};
	}
	return f;
}

bool CloseFile(File f)
{
	if (close(f.handle) == -1)
	{
		LogPrint(LogLevelError, "File", "Failed to close file %k: %k.\n", f.path, GetPlatformError());
		return false;
	}
	return true;
}

void ReadFromFile(File f, s64 n, StringBuilder *sb, bool *err)
{
	auto totRead = 0;
	auto curRead = 0; // Maximum number of bytes that can be returned by a read. (Like size_t, but signed.)
	auto cursor = &sb[sb->length];
	ResizeStringBuilder(sb, sb->length + n);
	do
	{
		curRead = read(f.handle, cursor, n - totRead);
		totRead += curRead;
		cursor += curRead;
	} while (totRead < n && curRead != 0 && curRead != -1);
	if (curRead == -1)
	{
		LogPrint(LogLevelError, "File", "Failed to read file %k: %k.\n", f.path, GetPlatformError());
		*err = true;
		return;
	}
	else if (totRead != n)
	{
		LogPrint(LogLevelError, "File", "Failed to read file %k: could only read %lu bytes, but %lu bytes were requested.\n", f.path, totRead, n);
		*err = true;
		return;
	}
}

bool WriteToFile(File f, s64 n, const void *buf)
{
	size_t totWrit = 0;
	ssize_t curWrit = 0; // Maximum number of bytes that can be returned by a write. (Like size_t, but signed.)
	auto cursor = (u8 *)buf;
	do
	{
		curWrit = write(f.handle, cursor, (n - totWrit));
		totWrit += curWrit;
		cursor += curWrit;
	} while (totWrit < n && curWrit != 0);
	if (totWrit != n)
	{
		LogPrint(LogLevelError, "File", "Failed to write file %k: %k.\n", f.path, GetPlatformError());
		return false;
	}
	return true;
}

// @TODO: Handle files larger than 2GB?
FileOffset FileLength(File f, bool *err)
{
	auto stat = (struct stat){};
	if (fstat(f.handle, &stat) == -1)
	{
		LogPrint(LogLevelError, "File", "Failed get length of file %k: %k.\n", f.path, GetPlatformError());
		*err = true;
		return {};
	}
	return FileOffset{stat.st_size};
}

FileOffset SeekInFile(File f, FileOffset seek, FileSeekRelative rel, bool *err)
{
	auto off = lseek(f.handle, seek, (s32)rel);
	if (off == (off_t)-1)
	{
		LogPrint(LogLevelError, "File", "Failed to seek file %k: %k.\n", f.path, GetPlatformError());
		*err = true;
		return {};
	}
	return off;
}

PlatformTime FileLastModifiedTime(File f, bool *err)
{
	auto stat = (struct stat){};
	if (fstat(f.handle, &stat) == -1)
	{
		LogPrint(LogLevelError, "File", "Failed to get last modified time of file %k: %k.\n", f.path, GetPlatformError());
		*err = true;
		return {};
	}
	return PlatformTime{stat.st_mtim};
}

bool IterateDirectory(DirectoryIteration *itr, String path)
{
	if (!itr->dir)
	{
		itr->dir = opendir(&path[0]);
		if (!itr->dir)
		{
			LogPrint(LogLevelError, "File", "Failed to open directory %k: %k.\n", path, GetPlatformError());
			return false;
		}
	}
	while ((itr->dirent = readdir(itr->dir)))
	{
		if (CStringsEqual(itr->dirent->d_name, ".") || CStringsEqual(itr->dirent->d_name, ".."))
		{
			continue;
		}
		itr->filename = itr->dirent->d_name;
		itr->isDir = (itr->dirent->d_type == DT_DIR);
		return true;
	}
	return false;
}

bool FileExists(String path)
{
	if (access(&path[0], F_OK) != -1)
	{
		return true;
	}
	return false;
}

bool CreateDirectoryIfItDoesNotExist(String path)
{
	if (FileExists(path))
	{
		return true;
	}
	if (mkdir(&path[0], 0700) == -1)
	{
		LogPrint(LogLevelError, "File", "Failed to create directory %k: %k.\n", path, GetPlatformError());
		return false;
	}
	return true;
}

bool DeleteFile(String path)
{
	if (unlink(&path[0]) != 0)
	{
		LogPrint(LogLevelError, "File", "Failed to delete file %k: %k.\n", path, GetPlatformError());
		return false;
	}
	return true;
}
