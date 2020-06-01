#include "../File.h"
#include "../Log.h"

FileHandle OpenFile(const String &path, OpenFileFlags flags, bool *error)
{
	auto file = open(&path[0], flags, 0666);
	if (file < 0)
	{
		LogPrint(ERROR_LOG, "Could not open file: %s.\n", &path[0]);
		*error = true;
		return {};
	}
	*error = false;
	return file;
}

bool CloseFile(FileHandle file)
{
	auto result = close(file);
	if (result == -1)
	{
		LogPrint(ERROR_LOG, "Could not close file: %s.\n", GetPlatformError());
		return false;
	}
	return true;
}

String ReadFromFile(FileHandle file, s64 byteCount, bool *error)
{
	auto totalBytesRead = 0;
	auto currentBytesRead = 0; // Maximum number of bytes that can be returned by a read. (Like size_t, but signed.)
	auto fileString = CreateString(byteCount);
	auto cursor = &fileString[0];
	do
	{
		currentBytesRead = read(file, cursor, byteCount - totalBytesRead);
		totalBytesRead += currentBytesRead;
		cursor += currentBytesRead;
	} while (totalBytesRead < byteCount && currentBytesRead != 0 && currentBytesRead != -1);
	if (currentBytesRead == -1)
	{
		LogPrint(ERROR_LOG, "ReadFromFile failed: could not read from file: %s.\n", GetPlatformError());
		*error = true;
		return "";
	}
	else if (totalBytesRead != byteCount)
	{
		// @TODO: Add file name to file handle.
		LogPrint(ERROR_LOG, "ReadFromFile failed: could only read %lu bytes, but %lu bytes were requested.\n", totalBytesRead, byteCount);
		*error = true;
		return "";
	}
	*error = false;
	return fileString;
}

bool WriteToFile(FileHandle file, s64 byteCount, void *buffer)
{
	size_t totalBytesWritten = 0;
	ssize_t currentBytesWritten = 0; // Maximum number of bytes that can be returned by a write. (Like size_t, but signed.)
	auto position = (char *)buffer;
	do
	{
		currentBytesWritten = write(file, position, (byteCount - totalBytesWritten));
		totalBytesWritten += currentBytesWritten;
		position += currentBytesWritten;
	} while (totalBytesWritten < byteCount && currentBytesWritten != 0);
	if (totalBytesWritten != byteCount)
	{
		// @TODO: Add file name to file handle.
		LogPrint(ERROR_LOG, "Could not write to file: %s.\n", GetPlatformError());
		return false;
	}
	return true;
}

FileOffset GetFileLength(FileHandle file, bool *error)
{
	struct stat stat; 
	if (fstat(file, &stat) == -1)
	{
		// @TODO: Add file name to file handle.
		LogPrint(ERROR_LOG, "Could not fstat file: %s.\n", GetPlatformError());
		*error = true;
		return {};
	}
	*error = false;
	return FileOffset{stat.st_size};
}

FileOffset SeekInFile(FileHandle file, FileOffset offset, FileSeekRelative relative, bool *error)
{
	auto result = lseek(file, offset, (s32)relative);
	if (result == (off_t)-1)
	{
		LogPrint(ERROR_LOG, "File seek failed: %s.\n", GetPlatformError());
		*error = true;
		return {};
	}
	*error = false;
	return result;
}

PlatformTime GetFileLastModifiedTime(FileHandle file, bool *error)
{
	struct stat stat;
	if (fstat(file, &stat) == -1)
	{
		LogPrint(ERROR_LOG, "Could not fstat file: %s.\n", GetPlatformError());
		*error = true;
		return {};
	}
	*error = false;
	return PlatformTime{stat.st_mtim};
}

bool IterateDirectory(const String &path, DirectoryIteration *context)
{
	if (!context->dir)
	{
		context->dir = opendir(&path[0]);
		if (!context->dir)
		{
			LogPrint(ERROR_LOG, "Failed to open directory %s: %s.\n", &path[0], GetPlatformError());
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

bool FileExists(const String &path)
{
	if (access(&path[0], F_OK) != -1)
	{
		return true;
	}
	return false;
}

bool CreateDirectoryIfItDoesNotExist(const String &path)
{
	if (FileExists(path))
	{
		return true;
	}
	if (mkdir(&path[0], 0700) == -1)
	{
		LogPrint(ERROR_LOG, "Failed to create directory %s: %s.\n", &path[0], GetPlatformError());
		return false;
	}
	return true;
}

bool DeleteFile(const String &path)
{
	auto returnCode = unlink(path.data.elements);
	if (returnCode != 0)
	{
		LogPrint(ERROR_LOG, "Failed to delete file %k: %k.\n", path, GetPlatformError());
		return false;
	}
	return true;
}
