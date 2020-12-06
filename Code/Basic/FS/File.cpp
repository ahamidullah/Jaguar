#include "File.h"
#include "Basic/String.h"

namespace fs
{

arr::array<u8> ReadAll(str::String path, bool *err)
{
	auto buf = arr::array<u8>{};
	auto f = Open(path, OpenFlags::ReadOnly, err);
	if (*err)
	{
		return {};
	}
	Defer(f.Close());
	auto fLen = f.Length(err);
	if (*err)
	{
		return {};
	}
	buf->Resize(buf->count + fLen);
	if (!f.Read(*buf))
	{
		*err = true;
		return {};
	}
	return buf;
}

}
