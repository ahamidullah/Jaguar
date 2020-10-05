#include "File.h"
#include "String.h"
#include "Time.h"

void ReadEntireFileIn(Array<u8> *a, String path, bool *err)
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
	a->Resize(a->count + flen);
	if (!f.Read(*a))
	{
		*err = true;
		return;
	}
}

Array<u8> ReadEntireFile(String path, bool *err)
{
	auto a = Array<u8>{};
	ReadEntireFileIn(&a, path, err);
	return a;
}
