#include "../File.h"
#include "../Log.h"

File OpenFile(String path, OpenFileFlags f, bool *error)
{
	auto file = File
	{
		.path = NewStringCopy(path),
	};
	file.handle = open(&path[0], f, 0666);
	if (file.handle < 0)
	{
		LogPrint(ERROR_LOG, "Failed to open file %k: %k.\n", path, GetPlatformError());
		*error = true;
		return {};
	}
	*error = false;
	return file;
}

bool CloseFile(File f)
{
	auto result = close(f.handle);
	if (result == -1)
	{
		LogPrint(ERROR_LOG, "Failed to close file %k: %k.\n", f.path, GetPlatformError());
		return false;
	}
	return true;
}

String ReadFromFile(File f, s64 count, bool *error)
{
	auto totalBytesRead = 0;
	auto currentBytesRead = 0; // Maximum number of bytes that can be returned by a read. (Like size_t, but signed.)
	auto fileString = NewString(count);
	auto cursor = &fileString[0];
	do
	{
		currentBytesRead = read(f.handle, cursor, count - totalBytesRead);
		totalBytesRead += currentBytesRead;
		cursor += currentBytesRead;
	} while (totalBytesRead < count && currentBytesRead != 0 && currentBytesRead != -1);
	if (currentBytesRead == -1)
	{
		LogPrint(ERROR_LOG, "Failed to read file %k: %k.\n", f.path, GetPlatformError());
		*error = true;
		return "";
	}
	else if (totalBytesRead != count)
	{
		LogPrint(ERROR_LOG, "Failed to read file %k: could only read %lu bytes, but %lu bytes were requested.\n", f.path, totalBytesRead, count);
		*error = true;
		return "";
	}
	*error = false;
	return fileString;
}

bool WriteToFile(File f, s64 count, void *buffer)
{
	size_t totalBytesWritten = 0;
	ssize_t currentBytesWritten = 0; // Maximum number of bytes that can be returned by a write. (Like size_t, but signed.)
	auto position = (char *)buffer;
	do
	{
		currentBytesWritten = write(f.handle, position, (count - totalBytesWritten));
		totalBytesWritten += currentBytesWritten;
		position += currentBytesWritten;
	} while (totalBytesWritten < count && currentBytesWritten != 0);
	if (totalBytesWritten != count)
	{
		LogPrint(ERROR_LOG, "Failed to write file %k: %k.\n", f.path, GetPlatformError());
		return false;
	}
	return true;
}

FileOffset GetFileLength(File f, bool *error)
{
	struct stat stat; 
	if (fstat(f.handle, &stat) == -1)
	{
		// @TODO: Add file name to file handle.
		LogPrint(ERROR_LOG, "Failed get length of file %k: %k.\n", GetPlatformError());
		*error = true;
		return {};
	}
	*error = false;
	return FileOffset{stat.st_size};
}

FileOffset SeekInFile(File f, FileOffset o, FileSeekRelative r, bool *error)
{
	auto result = lseek(f.handle, o, (s32)r);
	if (result == (off_t)-1)
	{
		LogPrint(ERROR_LOG, "Failed to seek file %k: %k.\n", f.path, GetPlatformError());
		*error = true;
		return {};
	}
	*error = false;
	return result;
}

PlatformTime GetFileLastModifiedTime(File f, bool *error)
{
	struct stat stat;
	if (fstat(f.handle, &stat) == -1)
	{
		LogPrint(ERROR_LOG, "Failed to get last modified time of file %k: %k.\n", GetPlatformError());
		*error = true;
		return {};
	}
	*error = false;
	return PlatformTime{stat.st_mtim};
}

bool IterateDirectory(DirectoryIteration *context, String path)
{
	if (!context->dir)
	{
		context->dir = opendir(&path[0]);
		if (!context->dir)
		{
			LogPrint(ERROR_LOG, "Failed to open directory %k: %k.\n", path, GetPlatformError());
			return false;
		}
	}
	while ((context->dirent = readdir(context->dir)))
	{
		if (CStringsEqual(context->dirent->d_name, ".") || CStringsEqual(context->dirent->d_name, ".."))
		{
			continue;
		}
		context->filename = context->dirent->d_name;
		context->isDirectory = (context->dirent->d_type == DT_DIR);
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
		LogPrint(ERROR_LOG, "Failed to create directory %k: %k.\n", path, GetPlatformError());
		return false;
	}
	return true;
}

bool DeleteFile(String path)
{
	if (unlink(&path[0]) != 0)
	{
		LogPrint(ERROR_LOG, "Failed to delete file %k: %k.\n", path, GetPlatformError());
		return false;
	}
	return true;
}
