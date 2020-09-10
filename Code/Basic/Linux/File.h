#pragma once

#include "../String.h"
#include "../Time.h"

enum FileSeekRelative
{
	FileSeekStart = SEEK_SET,
	FileSeekCurrent = SEEK_CUR,
	FileSeekEnd = SEEK_END,
};

struct File
{
	s64 handle;
	String path;

	bool Close();
	bool IsOpen();
	bool Write(ArrayView<u8> a);
	bool WriteString(String s);
	bool Read(ArrayView<u8> out);
	s64 Length(bool *err);
	s64 Seek(s64 seek, FileSeekRelative rel, bool *err);
	Time LastModifiedTime(bool *err);
};

struct DirectoryIteration
{
	DIR *dir;
	struct dirent *dirent;
	String filename;
	bool isDir;

	bool Iterate(String path);
};

enum OpenFileFlags
{
	OpenFileReadOnly = O_RDONLY,
	OpenFileWriteOnly = O_WRONLY,
	OpenFileReadWrite = O_RDWR,
	OpenFileCreate = O_CREAT | O_TRUNC,
};

File OpenFile(String path, s64 flags, bool *err);
bool FileExists(String path);
bool CreateDirectory(String path);
bool CreateDirectoryIfItDoesNotExist(String path);
bool DeleteFile(String path);
