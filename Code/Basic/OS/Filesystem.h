#pragma once

struct String;

String GetFilepathDirectory(const String &path);
String GetFilepathFilename(const String &path);
String GetFilepathExtension(const String &path);
void SetFilepathExtension(String *path, const String &newExtension);
String CleanFilepath(const String &filepath);

template <typename... StringPack>
String JoinFilepaths(StringPack... strings)
{
	auto stringCount = sizeof...(strings);
	if (stringCount == 0)
	{
		return "";
	}
	auto totalStringLengths = (strings.count + ...);
	auto totalFilepathLength = totalStringLengths + (stringCount - 1);
	auto filepath = CreateString(0, totalFilepathLength);
	auto AddPathComponent = [](String *path, String component)
	{
		StringAppend(path, component);
		StringAppend(path, "/");
	};
	(AddPathComponent(&filepath, strings), ...);
	return filepath;
}
