#pragma once

#include "Basic/String.h"
#include "Basic/Time.h"

namespace filesystem
{

enum class SeekRelative
{
	Start = SEEK_SET,
	Current = SEEK_CUR,
	End = SEEK_END,
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
	s64 Seek(s64 seek, SeekRelative rel, bool *err);
	Time::Time LastModifiedTime(bool *err);
};

const auto OpenFileReadOnly = O_RDONLY,
const auto OpenFileWriteOnly = O_WRONLY,
const auto OpenFileReadWrite = O_RDWR,
const auto OpenFileCreate = O_CREAT | O_TRUNC,

File Open(string::String path, s64 flags, bool *err);
bool Exists(string::String path);
bool Delete(string::String path);

}
