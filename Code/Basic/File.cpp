#include "File.h"
#include "String.h"
#include "Time.h"

void ReadEntireFileIn(Array<u8> *buf, String path, bool *err)
{
	auto f = OpenFile(path, OpenFileReadOnly, err);
	if (*err)
	{
		return;
	}
	Defer(f.Close());
	auto fLen = f.Length(err);
	if (*err)
	{
		return;
	}
	buf->Resize(buf->count + fLen);
	if (!f.Read(*buf))
	{
		*err = true;
		return;
	}
}

Array<u8> ReadEntireFile(String path, bool *err)
{
	auto buf = Array<u8>{};
	ReadEntireFileIn(&buf, path, err);
	return buf;
}
