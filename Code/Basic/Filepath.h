#pragma once

#include "String.h"

void FilepathDirectoryIn(StringBuilder *sb, String path);
String FilepathDirectory(String path);
void FilepathFilenameIn(StringBuilder *sb, String path);
String FilepathFilename(String path);
void FilepathExtensionIn(StringBuilder *sb, String path);
String FilepathExtension(String path);
void SetFilepathExtensionIn(StringBuilder *path, String ext);
String SetFilepathExtension(String path, String ext);

template <typename... StringPack>
String JoinFilepaths(StringPack... sp)
{
	auto count = sizeof...(sp);
	if (count == 0)
	{
		return "";
	}
	auto len = (String{sp}.Length() + ...) + (count - 1);
	auto sb = NewStringBuilderWithCapacity(len);
	auto AddPathComponent = [](StringBuilder *sb, String c)
	{
		sb->Append(c);
		sb->Append("/");
	};
	(AddPathComponent(&sb, sp), ...);
	sb.Resize(sb.Length() - 1); // Get rid of the last extraneous '/'.
	return NewStringFromBuffer(sb.buffer);
}
