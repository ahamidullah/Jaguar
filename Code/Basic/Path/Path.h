#pragma once

#include "Basic/String.h"

namespace path
{

str::View Directory(str::View path);
str::View Extension(str::View path);
str::View Filename(str::View path);
str::View Filestem(str::View path);

template <typename... StringPack>
str::String Join(StringPack... sp)
{
	auto count = sizeof...(sp);
	if (count == 0)
	{
		return "";
	}
	auto b = Builder{};
	(b.Append(sp), ...);
	return str::NewFromBuffer(b.buffer);
}

}
