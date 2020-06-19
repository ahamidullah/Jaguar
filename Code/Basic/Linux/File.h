#pragma once

#include "../String.h"

struct File
{
	s32 handle;
	String path;
};

typedef off_t FileOffset;

struct DirectoryIteration
{
	DIR *dir = NULL;
	struct dirent *dirent = NULL;
	String filename = "";
	bool isDirectory = false;
};

enum FileSeekRelative
{
	FILE_SEEK_RELATIVE_TO_START = SEEK_SET,
	FILE_SEEK_RELATIVE_TO_CURRENT = SEEK_CUR,
	FILE_SEEK_RELATIVE_TO_END = SEEK_END,
};

typedef s64 OpenFileFlags;
enum OpenFileFlagBits
{
	OPEN_FILE_READ_ONLY = O_RDONLY,
	OPEN_FILE_WRITE_ONLY = O_WRONLY,
	OPEN_FILE_CREATE = O_CREAT | O_TRUNC,
};

struct PlatformTime;

File OpenFile(String path, OpenFileFlags f, bool *error);
bool CloseFile(File f);
String ReadFromFile(File f, s64 count, bool *error);
bool WriteToFile(File f, s64 count, void *buffer);
FileOffset GetFileLength(File f, bool *error);
FileOffset SeekInFile(File f, FileOffset o, FileSeekRelative r, bool *error);
PlatformTime GetFileLastModifiedTime(File f, bool *error);
bool IterateDirectory(DirectoryIteration *context, String path);
bool FileExists(String path);
bool CreateDirectoryIfItDoesNotExist(String path);
bool DeleteFile(String path);
