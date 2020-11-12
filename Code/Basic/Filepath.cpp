#include "Filepath.h"
#include "Basic/String.h"

// FilepathDirectory returns all but the last component of the path.
void FilepathDirectoryIn(string::Builder *sb, string::String path)
{
	auto slash = path.FindLast('/');
	if (slash < 0)
	{
		return;
	}
	return sb->Append(path.ToView(0, slash));
}

string::String FilepathDirectory(string::String path)
{
	auto sb = string::Builder{};
	FilepathDirectoryIn(&sb, path);
	return string::NewFromBuffer(sb.buffer);
}

// FilepathFilename returns the last component of the path.
void FilepathFilenameIn(string::Builder *sb, string::String path)
{
	auto slash = path.FindLast('/');
	if (slash < 0)
	{
		sb->Append(path);
		return;
	}
	sb->Append(path.ToView(slash + 1, path.Length()));
}

string::String FilepathFilename(string::String path)
{
	auto sb = string::Builder{};
	FilepathFilenameIn(&sb, path);
	return string::NewFromBuffer(sb.buffer);
}

void FilepathFilenameNoExtIn(string::Builder *sb, string::String path)
{
	auto start = 0;
	auto slash = path.FindLast('/');
	if (slash < 0)
	{
		start = 0;
	}
	else
	{
		start = slash + 1;
	}
	auto dot = path.FindLast('.');
	if (dot <= start)
	{
		return;
	}
	Assert(dot > 0);
	sb->Append(path.ToView(start, dot));
}

string::String FilepathFilenameNoExt(string::String path)
{
	auto sb = string::Builder{};
	FilepathFilenameNoExtIn(&sb, path);
	return string::NewFromBuffer(sb.buffer);
}

// FilepathExtension returns the file extension including the dot.
void FilepathExtensionIn(string::Builder *sb, string::String path)
{
	auto dot = path.FindLast('.');
	if (dot < 0)
	{
		return;
	}
	return sb->Append(path.ToView(dot, path.Length()));
}

string::String FilepathExtension(string::String path)
{
	auto sb = string::Builder{};
	FilepathExtensionIn(&sb, path);
	return string::NewFromBuffer(sb.buffer);
}

// SetFilepathExtension replaces the portion of the path after the last dot character with the supplied extension.
// If no dot character is found, a dot character and the supplied extension are appended to the path.
void SetFilepathExtensionIn(string::Builder *path, string::String ext)
{
	auto dot = path->FindLast('.');
	if (dot < 0)
	{
		path->Append(".");
		path->Append(ext);
		return;
	}
	path->Resize(dot + ext.Length());
	array::Copy(ext.buffer, path->ToView(dot, path->Length()).buffer);
}

string::String SetFilepathExtension(string::String path, string::String ext)
{
	auto sb = string::NewBuilderWithCapacity(path.Length());
	sb.Append(path);
	SetFilepathExtensionIn(&sb, ext);
	return string::NewFromBuffer(sb.buffer);
}

#if 0
// JoinFilepaths concatenates two strings and inserts a '/' between them.
string::Builder JoinFilepaths(string::String a, string::String b)
{
	auto sb = Newstring::BuilderWithCapacity(a.length + b.length + 1);
	string::BuilderAppend(a);
	string::BuilderAppend("/");
	string::BuilderAppend(b);
	return sb;
}

// This is probably buggy/full of corner cases, but oh well!
// @TODO: Do we still use this?
// Don't remember what this does lol.
string::String CleanFilepath(const string::String &filepath)
{
	auto components = Splitstring::String(filepath, '/');
	for (auto i = 0; i < ArrayLength(components); i += 1)
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
	auto result = Createstring::String(0, string::StringLength(filepath));
	if (string::StringLength(filepath) > 0 && filepath[0] == '/')
	{
		string::StringAppend(&result, "/");
	}
	for (auto i = 0; i < ArrayLength(components); i += 1)
	{
		string::StringAppend(&result, components[i]);
		if (i != ArrayLength(components) - 1)
		{
			string::StringAppend(&result, "/");
		}
	}
	return result;
}
#endif
