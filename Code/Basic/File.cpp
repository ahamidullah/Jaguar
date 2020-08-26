#include "File.h"
#include "String.h"
#include "Time.h"

bool ReadEntireFileIn(StringBuilder *sb, String path)
{
	auto err = false;
	auto f = OpenFile(path, OpenFileReadOnly, &err);
	if (err)
	{
		return false;
	}
	Defer(f.Close());
	auto flen = f.Length(&err);
	if (err)
	{
		return false;
	}
	auto i = sb->Length();
	sb->Resize(sb->Length() + flen);
	if (!f.Read(sb->ToView(i, sb->Length()).buffer))
	{
		return false;
	}
	return true;
}

String ReadEntireFile(String path, bool *err)
{
	auto sb = StringBuilder{};
	*err = ReadEntireFileIn(&sb, path);
	return NewStringFromBytes(sb.buffer);
}
