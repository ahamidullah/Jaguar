#pragma once

#include "String.h"

String FilepathDirectory(String path);
String FilepathFilename(String path);
String FilepathExtension(String path);
void SetFilepathExtension(StringBuilder *path, String ext);

template <typename... StringPack>
String JoinFilepaths(StringPack... sp)
{
	auto count = sizeof...(sp);
	if (count == 0)
	{
		return "";
	}
	auto len = (sp.length + ...) + (count - 1);
	auto buf = (char *)AllocateMemory(len + 1);
	auto i = 0;
	auto AddPathComponent = [&i](char *b, String c)
	{
		CopyMemory(c.buffer, &b[i], c.length);
		b[i + c.length] = '/';
		i += c.length + 1;
	};
	(AddPathComponent(buf, sp), ...);
	return
	{
		.allocator = ContextAllocator(),
		.buffer = buf,
		.length = len,
		.literal = false,
	};
}
