#pragma once

#include <dirent.h>
#include <fcntl.h>

#include "Basic/String.h"

struct OpenFileResult;
struct ReadFileResult;
struct String;

typedef s32 FileHandle;
typedef u64 FileOffset;

constexpr FileHandle FILE_HANDLE_ERROR = -1;
constexpr FileOffset FILE_OFFSET_ERROR = (FileOffset)-1;

struct DirectoryIteration {
	DIR *dir = NULL;
	struct dirent *dirent = NULL;
	String filename;
	u8 isDirectory = 0;
};

enum FileSeekRelative {
	FILE_SEEK_RELATIVE_TO_START = SEEK_SET,
	FILE_SEEK_RELATIVE_TO_CURRENT = SEEK_CUR,
	FILE_SEEK_RELATIVE_TO_END = SEEK_END,
};

enum OpenFileFlags {
	OPEN_FILE_READ_ONLY = O_RDONLY,
};

enum {
	STANDARD_OUT = 1,
	STANDARD_IN = 0,
	STANDARD_ERROR = 2,
};

OpenFileResult OpenFile(const String &path, OpenFileFlags flags);
bool CloseFile(FileHandle file);
ReadFileResult ReadFile(FileHandle file, size_t numberOfBytesToRead);
bool WriteFile(FileHandle file, size_t count, const void *buffer);
FileOffset GetFileLength(FileHandle file);
FileOffset SeekFile(FileHandle file, FileOffset offset, FileSeekRelative relative);
bool IterateDirectory(const String &path, DirectoryIteration *context);
PlatformTime GetFileLastModifiedTime(FileHandle file);
bool FileExists(const String &path);
