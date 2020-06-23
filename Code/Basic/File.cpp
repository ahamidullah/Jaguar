#include "File.h"
#include "String.h"
#include "Time.h"

void ReadEntireFile(String path, StringBuilder *sb, bool *err)
{
	auto f = OpenFile(path, OpenFileReadOnly, err);
	if (*err)
	{
		return;
	}
	Defer(CloseFile(f));
	auto l = FileLength(f, err);
	if (*err)
	{
		return;
	}
	ReadFromFile(f, l, sb, err);
}

PlatformTime GetFilepathLastModifiedTime(String path, bool *err)
{
	auto f = OpenFile(path, OpenFileReadOnly, err);
	if (*err)
	{
		return {};
	}
	Defer(CloseFile(f));
	auto t = FileLastModifiedTime(f, err);
	if (*err)
	{
		return {};
	}
	return t;
}
