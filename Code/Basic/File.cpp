#include "File.h"
#include "String.h"
#include "Time.h"

void ReadEntireFileIn(StringBuilder *sb, String path, bool *err)
{
	auto f = OpenFile(path, OpenFileReadOnly, err);
	if (*err)
	{
		return;
	}
	Defer(f.Close());
	auto flen = f.Length(err);
	if (*err)
	{
		return;
	}
	auto i = sb->Length();
	sb->Resize(sb->Length() + flen);
	if (!f.Read(sb->View(i, sb->Length()).buffer))
	{
		*err = true;
		return;
	}
}

String ReadEntireFile(String path, bool *err)
{
	auto sb = StringBuilder{};
	ReadEntireFileIn(&sb, path, err);
	return NewStringFromBuffer(sb.buffer);
}
