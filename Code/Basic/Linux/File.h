#pragma once

#include "../String.h"
#include "../Time.h"

struct File
{
	s64 handle;
	String path;
};

typedef off_t FileOffset;

struct DirectoryIteration
{
	DIR *dir;
	struct dirent *dirent;
	String filename;
	bool isDir;
};

enum FileSeekRelative
{
	FileSeekStart = SEEK_SET,
	FileSeekCurrent = SEEK_CUR,
	FileSeekEnd = SEEK_END,
};

enum OpenFileFlags
{
	OpenFileReadOnly = O_RDONLY,
	OpenFileWriteOnly = O_WRONLY,
	OpenFileReadWrite = O_RDWR,
	OpenFileCreate = O_CREAT | O_TRUNC,
};

File OpenFile(String path, s64 flags, bool *err);
bool CloseFile(File f);
void ReadFromFile(File f, s64 n, StringBuilder *sb, bool *err);
bool WriteToFile(File f, s64 count, const void *buffer);
FileOffset FileLength(File f, bool *err);
FileOffset SeekInFile(File f, FileOffset seek, FileSeekRelative rel, bool *err);
PlatformTime FileLastModifiedTime(File f, bool *err);
bool IterateDirectory(DirectoryIteration *context, String path);
bool FileExists(String path);
bool CreateDirectory(String path);
bool CreateDirectoryIfItDoesNotExist(String path);
bool DeleteFile(String path);
