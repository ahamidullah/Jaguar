#include <sys/stat.h>

OpenFileResult OpenFile(const String &path, OpenFileFlags flags)
{
	auto file = open(&path[0], (s32)flags, 0666);
	if (file < 0)
	{
		LogPrint(LogType::ERROR, "could not open file: %s\n", &path[0]);
		return {.error = true};
	}
	return {.file = file};
}

bool CloseFile(FileHandle file)
{
	auto result = close(file);
	if (result == -1)
	{
		LogPrint(LogType::ERROR, "could not close file: %s\n", GetPlatformError());
		return false;
	}
	return true;
}

ReadFileResult ReadFile(FileHandle file, size_t numberOfBytesToRead)
{
	ReadFileResult result;
	size_t totalBytesRead = 0;
	ssize_t currentBytesRead = 0; // Maximum number of bytes that can be returned by a read. (Like size_t, but signed.)
	auto fileString = CreateString(numberOfBytesToRead);
	auto *cursor = &fileString[0];
	do
	{
		currentBytesRead = read(file, cursor, numberOfBytesToRead - totalBytesRead);
		totalBytesRead += currentBytesRead;
		cursor += currentBytesRead;
	} while (totalBytesRead < numberOfBytesToRead && currentBytesRead != 0 && currentBytesRead != -1);
	if (currentBytesRead == -1)
	{
		LogPrint(LogType::ERROR, "ReadFile failed: could not read from file: %s\n", GetPlatformError());
		result.error = true;
		return result;
	}
	else if (totalBytesRead != numberOfBytesToRead)
	{
		// @TODO: Add file name to file handle.
		LogPrint(LogType::ERROR, "ReadFromFile failed: could only read %lu bytes, but %lu bytes were requested\n", totalBytesRead, numberOfBytesToRead);
		result.error = true;
		return result;
	}
	result.string = fileString;
	return result;
}

bool WriteFile(FileHandle file, size_t count, const void *buffer)
{
	size_t totalBytesWritten = 0;
	ssize_t currentBytesWritten = 0; // Maximum number of bytes that can be returned by a write. (Like size_t, but signed.)
	auto position = (char *)buffer;
	do
	{
		currentBytesWritten = write(file, position, (count - totalBytesWritten));
		totalBytesWritten += currentBytesWritten;
		position += currentBytesWritten;
	} while (totalBytesWritten < count && currentBytesWritten != 0);
	if (totalBytesWritten != count)
	{
		// @TODO: Add file name to file handle.
		LogPrint(LogType::ERROR, "Could not write to file: %s\n", GetPlatformError());
		return false;
	}
	return true;
}

FileOffset GetFileLength(FileHandle file)
{
	struct stat stat; 
	if (fstat(file, &stat) == 0)
	{
		return (FileOffset)stat.st_size;
	}
	return FILE_OFFSET_ERROR; 
}

FileOffset SeekFile(FileHandle file, FileOffset offset, FileSeekRelative relative)
{
	auto result = lseek(file, offset, (s32)relative);
	if (result == (off_t)-1)
	{
		LogPrint(LogType::ERROR, "File seek failed: %s\n", GetPlatformError());
	}
	return result;
}

PlatformTime GetFileLastModifiedTime(FileHandle file)
{
	struct stat stat;
	auto errorCode = fstat(file, &stat);
	if (errorCode == -1)
	{
		LogPrint(LogType::ERROR, "failed fstat: %s\n", GetPlatformError());
		return PLATFORM_TIME_ERROR;
	}
	return PlatformTime{stat.st_mtim};
}

bool IterateDirectory(const String &path, DirectoryIteration *context)
{
	if (!context->dir)
	{
		context->dir = opendir(&path[0]);
		if (!context->dir)
		{
			LogPrint(LogType::ERROR, "failed to open directory %s: %s\n", &path[0], GetPlatformError());
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
