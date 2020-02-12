#include <sys/stat.h>

#include "Platform/Files.h"

PlatformOpenFileResult PlatformOpenFile(const String &path, PlatformOpenFileFlags flags)
{
	auto file = open(&path[0], flags, 0666);
	if (file < 0)
	{
		LogPrint(ERROR_LOG, "could not open file: %s", &path[0]);
		return {.error = true};
	}
	return {.file = file};
}

bool PlatformCloseFile(PlatformFileHandle file) {
	s32 result = close(file);
	if (result == -1)
	{
		LogPrint(ERROR_LOG, "Could not close file: %s", PlatformGetError());
		return false;
	}
	return true;
}

PlatformReadFileResult PlatformReadFromFile(PlatformFileHandle file, size_t numberOfBytesToRead)
{
	PlatformReadFileResult result;
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
		LogPrint(ERROR_LOG, "ReadFromFile failed: could not read from file: %s", PlatformGetError());
		result.error = true;
		return result;
	}
	else if (totalBytesRead != numberOfBytesToRead)
	{
		// @TODO: Add file name to file handle.
		LogPrint(ERROR_LOG, "ReadFromFile failed: could only read %lu bytes, but %lu bytes were requested", totalBytesRead, numberOfBytesToRead);
		result.error = true;
		return result;
	}
	result.string = fileString;
	return result;
}

bool PlatformWriteToFile(PlatformFileHandle file, size_t count, const void *buffer)
{
	size_t totalBytesWritten = 0;
	ssize_t currentBytesWritten = 0; // Maximum number of bytes that can be returned by a write. (Like size_t, but signed.)
	const char *position = (char *)buffer;
	do
	{
		currentBytesWritten = write(file, position, (count - totalBytesWritten));
		totalBytesWritten += currentBytesWritten;
		position += currentBytesWritten;
	} while (totalBytesWritten < count && currentBytesWritten != 0);
	if (totalBytesWritten != count)
	{
		// @TODO: Add file name to file handle.
		LogPrint(ERROR_LOG, "Could not write to file: %s", PlatformGetError());
		return false;
	}
	return true;
}

PlatformFileOffset PlatformGetFileLength(PlatformFileHandle file)
{
	struct stat stat; 
	if (fstat(file, &stat) == 0)
	{
		return (PlatformFileOffset)stat.st_size;
	}
	return FILE_OFFSET_ERROR; 
}

PlatformFileOffset PlatformSeekInFile(PlatformFileHandle file, PlatformFileOffset offset, PlatformFileSeekRelative relative)
{
	auto result = lseek(file, offset, relative);
	if (result == (off_t)-1)
	{
		LogPrint(ERROR_LOG, "File seek failed: %s", PlatformGetError());
	}
	return result;
}

bool PlatformIterateThroughDirectory(const char *path, PlatformDirectoryIteration *context)
{
	if (!context->dir)
	{
		context->dir = opendir(path);
		if (!context->dir)
		{
			LogPrint(ERROR_LOG, "Failed to open directory %s: %s\n", path, PlatformGetError());
			return false;
		}
	}
	while ((context->dirent = readdir(context->dir)))
	{
		// @TODO
		if (strcmp(context->dirent->d_name, ".") == 0 || strcmp(context->dirent->d_name, "..") == 0)
		{
			continue;
		}
		context->filename = context->dirent->d_name;
		context->isDirectory = (context->dirent->d_type == DT_DIR);
		return true;
	}
	return false;
}
