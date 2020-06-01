#include "Filesystem.h"
#include "String.h"

// Returns all but the last component of the path.
String GetFilepathDirectory(const String &path)
{
	auto slashIndex = FindLastCharIndex(path, '/');
	if (slashIndex < 0)
	{
		return "";
	}
	auto directoryLength = slashIndex;
	auto directory = CreateString(directoryLength);
	CopyMemory(&path[0], &directory[0], directoryLength);
	return directory;
}

// Returns the last component of the path.
String GetFilepathFilename(const String &path)
{
	auto slashIndex = FindLastCharIndex(path, '/');
	if (slashIndex < 0)
	{
		return CreateString(path);
	}
	auto filenameLength = StringLength(path) - (slashIndex + 1);
	auto filename = CreateString(filenameLength);
	CopyMemory(&path[0] + slashIndex + 1, &filename[0], filenameLength);
	return filename;
}

// Returns the file extension including the dot.
String GetFilepathExtension(const String &path)
{
	auto dotIndex = FindLastCharIndex(path, '.');
	if (dotIndex < 0)
	{
		return "";
	}
	auto fileExtensionLength = StringLength(path) - dotIndex;
	auto fileExtension = CreateString(fileExtensionLength);
	CopyMemory(&path[0] + dotIndex, &fileExtension[0], fileExtensionLength);
	return fileExtension;
}

void SetFilepathExtension(String *path, const String &extension)
{
	auto dotIndex = FindLastCharIndex(*path, '.');
	if (dotIndex < 0)
	{
		StringAppend(path, ".");
		StringAppend(path, extension);
		return;
	}
	auto extensionLength = StringLength(extension);
	ResizeString(path, dotIndex + extensionLength);
	CopyMemory(&extension[0], &(*path)[dotIndex], extensionLength);
}

// Concatenates two strings and inserts a '/' between them.
String JoinFilepaths(const String &a, const String &b)
{
	auto result = CreateString(StringLength(a) + StringLength(b) + 1);
	CopyMemory(&a[0], &result[0], StringLength(a));
	result[StringLength(a)] = '/';
	CopyMemory(&b[0], &result[0] + StringLength(a) + 1, StringLength(b));
	return result;
}

// This is probably buggy/full of corner cases, but oh well!
String CleanFilepath(const String &filepath)
{
	auto components = SplitString(filepath, '/');
	for (auto i = 0; i < ArrayLength(components); i++)
	{
		if (components[i] == ".")
		{
			RemoveArrayElement(&components, i);
		}
		if (components[i] == "..")
		{
			RemoveArrayElement(&components, i);
			if (i != 0)
			{
				RemoveArrayElement(&components, i - 1);
			}
		}
	}
	auto result = CreateString(0, StringLength(filepath));
	if (StringLength(filepath) > 0 && filepath[0] == '/')
	{
		StringAppend(&result, "/");
	}
	for (auto i = 0; i < ArrayLength(components); i++)
	{
		StringAppend(&result, components[i]);
		if (i != ArrayLength(components) - 1)
		{
			StringAppend(&result, "/");
		}
	}
	return result;
}
