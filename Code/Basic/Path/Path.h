#pragma once

#include "Basic/String.h"

namespace path
{

string::View Directory(string::String path);
string::View Extension(string::String path);
string::View Filename(string::String path);
string::View Filestem(string::String path);

template <typename... StringPack>
string::String Join(StringPack... sp)
{
	auto count = sizeof...(sp);
	if (count == 0)
	{
		return "";
	}
	auto b = Builder{};
	(b.Append(sp), ...);
	return string::NewFromBuffer(b.buffer);
}

}
