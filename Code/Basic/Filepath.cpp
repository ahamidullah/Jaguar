#include "Filepath.h"
#include "String.h"

// FilepathDirectory returns all but the last component of the path.
void FilepathDirectory(StringBuilder *sb, String path)
{
	auto slash = FindLastCharInString(path, '/');
	if (slash < 0)
	{
		return;
	}
	return AppendRangeToStringBuilder(sb, path, 0, slash);
}

// FilepathFilename returns the last component of the path.
void FilepathFilename(StringBuilder *sb, String path)
{
	auto slash = FindLastCharInString(path, '/');
	if (slash < 0)
	{
		AppendToStringBuilder(sb, path);
		return;
	}
	AppendRangeToStringBuilder(sb, path, slash + 1, path.length - (slash + 1));
}

// FilepathExtension returns the file extension including the dot.
void FilepathExtension(StringBuilder *sb, String path)
{
	auto dot = FindLastCharInString(path, '.');
	if (dot < 0)
	{
		return;
	}
	return AppendRangeToStringBuilder(sb, path, dot, path.length - dot);
}

// SetFilepathExtension replaces the portion of the path after the last dot character with the supplied extension.
// If no dot character is found, a dot character and the supplied extension are appended to the path.
void SetFilepathExtension(StringBuilder *path, String ext)
{
	auto dot = FindLastCharInStringBuilder(*path, '.');
	if (dot < 0)
	{
		AppendToStringBuilder(path, ".");
		AppendToStringBuilder(path, ext);
		return;
	}
	ResizeStringBuilder(path, dot + ext.length);
	CopyMemory(&ext[0], &(*path)[dot], ext.length);
}

#if 0
// JoinFilepaths concatenates two strings and inserts a '/' between them.
StringBuilder JoinFilepaths(String a, String b)
{
	auto sb = NewStringBuilderWithCapacity(a.length + b.length + 1);
	StringBuilderAppend(a);
	StringBuilderAppend("/");
	StringBuilderAppend(b);
	return sb;
}

// This is probably buggy/full of corner cases, but oh well!
// @TODO: Do we still use this?
// Don't remember what this does lol.
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
#endif
