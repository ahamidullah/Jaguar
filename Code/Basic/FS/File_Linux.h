#pragma once

#include "Basic/Str.h"
#include "Basic/Time.h"

namespace fs
{

enum class SeekRelative
{
	Start = SEEK_SET,
	Current = SEEK_CUR,
	End = SEEK_END,
};

const auto OpenFileReadOnly = O_RDONLY,
const auto OpenFileWriteOnly = O_WRONLY,
const auto OpenFileReadWrite = O_RDWR,
const auto OpenFileCreate = O_CREAT | O_TRUNC,

struct File
{
	s64 handle;
	str::String path;

	bool Close();
	bool IsOpen();
	bool Write(arr::View<u8> a);
	bool WriteString(str::String s);
	bool Read(arr::View<u8> out);
	s64 Length(bool *err);
	s64 Seek(s64 seek, SeekRelative rel, bool *err);
	Time::Time LastModifiedTime(bool *err);
};

File Create(str::String path);
File Open(str::String path, s64 flags, bool *err);
bool Exists(str::String path);
bool Delete(str::String path);

}
