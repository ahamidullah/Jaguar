#include "File.h"
#include "Basic/String.h"

void ReadEntireFileIn(array::Array<u8> *buf, string::String path, bool *err)
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

array::Array<u8> ReadEntireFile(string::String path, bool *err)
{
	auto buf = array::Array<u8>{};
	ReadEntireFileIn(&buf, path, err);
	return buf;
}
