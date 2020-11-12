#pragma once

#include "Basic/String.h"

void FilepathDirectoryIn(string::Builder *sb, string::String path);
string::String FilepathDirectory(string::String path);
void FilepathFilenameIn(string::Builder *sb, string::String path);
string::String FilepathFilename(string::String path);
void FilepathExtensionIn(string::Builder *sb, string::String path);
string::String FilepathExtension(string::String path);
void SetFilepathExtensionIn(string::Builder *path, string::String ext);
string::String SetFilepathExtension(string::String path, string::String ext);
void FilepathFilenameNoExtIn(string::Builder *sb, string::String path);
string::String FilepathFilenameNoExt(string::String path);

template <typename... StringPack>
string::String JoinFilepaths(StringPack... sp)
{
	auto count = sizeof...(sp);
	if (count == 0)
	{
		return "";
	}
	auto len = (string::Length(sp) + ...) + (count - 1);
	auto sb = string::NewBuilderWithCapacity(len);
	auto AddPathComponent = [](string::Builder *sb, string::String c)
	{
		sb->Append(c);
		sb->Append("/");
	};
	(AddPathComponent(&sb, sp), ...);
	sb.Resize(sb.Length() - 1); // Get rid of the last extraneous '/'.
	return string::NewFromBuffer(sb.buffer);
}
