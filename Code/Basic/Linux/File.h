#pragma once

#include "Basic/String.h"
#include "Basic/Time/Time.h"

enum FileSeekRelative
{
	FileSeekStart = SEEK_SET,
	FileSeekCurrent = SEEK_CUR,
	FileSeekEnd = SEEK_END,
};

struct File
{
	s64 handle;
	string::String path;

	bool Close();
	bool IsOpen();
	bool Write(array::View<u8> a);
	bool WriteString(string::String s);
	bool Read(array::View<u8> out);
	s64 Length(bool *err);
	s64 Seek(s64 seek, FileSeekRelative rel, bool *err);
	Time::Time LastModifiedTime(bool *err);
};

struct DirectoryIteration
{
	DIR *dir;
	struct dirent *dirent;
	string::String filename;
	bool isDirectory;

	bool Iterate(string::String path);
};

enum OpenFileFlags
{
	OpenFileReadOnly = O_RDONLY,
	OpenFileWriteOnly = O_WRONLY,
	OpenFileReadWrite = O_RDWR,
	OpenFileCreate = O_CREAT | O_TRUNC,
};

File OpenFile(string::String path, s64 flags, bool *err);
bool FileExists(string::String path);
bool CreateDirectory(string::String path);
bool CreateDirectoryIfItDoesNotExist(string::String path);
bool DeleteFile(string::String path);
