#pragma once

#include <dirent.h>
#include <fcntl.h>

#include "Basic/String.h"

typedef s32 FileHandle;
typedef u64 FileOffset;

struct DirectoryIteration
{
	DIR *dir = NULL;
	struct dirent *dirent = NULL;
	String filename;
	bool isDirectory = 0;
};

enum FileSeekRelative
{
	FILE_SEEK_RELATIVE_TO_START = SEEK_SET,
	FILE_SEEK_RELATIVE_TO_CURRENT = SEEK_CUR,
	FILE_SEEK_RELATIVE_TO_END = SEEK_END,
};

typedef u32 OpenFileFlags;
enum OpenFileFlagBits
{
	OPEN_FILE_READ_ONLY = O_RDONLY,
	OPEN_FILE_WRITE_ONLY = O_WRONLY,
	OPEN_FILE_CREATE = O_CREAT | O_TRUNC,
};

FileHandle OpenFile(const String &path, OpenFileFlags flags);
bool CloseFile(FileHandle file);
String ReadFromFile(FileHandle file, size_t numberOfBytesToRead, bool *error);
bool WriteToFile(FileHandle file, size_t count, void *buffer);
FileOffset GetFileLength(FileHandle file);
FileOffset SeekFile(FileHandle file, FileOffset offset, FileSeekRelative relative);
bool IterateDirectory(const String &path, DirectoryIteration *context);
PlatformTime GetFileLastModifiedTime(FileHandle file);
bool FileExists(const String &path);
bool CreateDirectoryIfItDoesNotExist(const String &path);
