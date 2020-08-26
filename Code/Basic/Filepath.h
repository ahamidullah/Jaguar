#pragma once

#include "String.h"

void FilepathDirectory(StringBuilder *sb, String path);
void FilepathFilename(StringBuilder *sb, String path);
void FilepathExtension(StringBuilder *sb, String path);
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
	auto sb = NewStringBuilderWithCapacity(len);
	auto AddPathComponent = [](StringBuilder *sb, String c)
	{
		sb->Append(c);
		sb->Append("/");
	};
	(AddPathComponent(&sb, sp), ...);
	return {sb.buffer, false};
}
