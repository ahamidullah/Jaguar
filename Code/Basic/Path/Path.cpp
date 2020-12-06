#include "Path.h"
#include "Basic/String.h"

namespace path
{

// Directory returns all but the last component of the path.
str::View Directory(str::View path)
{
	auto slash = path.FindLast('/');
	if (slash < 0)
	{
		return;
	}
	return path.ToView(0, slash);
}

str::View Extension(str::View path)
{
	auto dot = path.FindLast('.');
	if (dot < 0)
	{
		return;
	}
	path.ToView(dot, path.Length());
}

// Filename returns the last component of the path.
str::View Filename(str::View path)
{
	auto slash = path.FindLast('/');
	if (slash < 0)
	{
		sb->Append(path);
		return;
	}
	return path.ToView(slash + 1, path.Length());
}

// Filestem returns the filename without its extension.
str::View Filestem(str::View path)
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
	return path.ToView(start, dot);
}

#if 0
// FilepathDirectory returns all but the last component of the path.
void FilepathDirectoryIn(str::Builder *sb, str::String path)
{
	auto slash = path.FindLast('/');
	if (slash < 0)
	{
		return;
	}
	return sb->Append(path.ToView(0, slash));
}

str::String FilepathDirectory(str::String path)
{
	auto sb = str::Builder{};
	FilepathDirectoryIn(&sb, path);
	return str::NewFromBuffer(sb.buffer);
}

// FilepathFilename returns the last component of the path.
void FilepathFilenameIn(str::Builder *sb, str::String path)
{
	auto slash = path.FindLast('/');
	if (slash < 0)
	{
		sb->Append(path);
		return;
	}
	sb->Append(path.ToView(slash + 1, path.Length()));
}

str::String FilepathFilename(str::String path)
{
	auto sb = str::Builder{};
	FilepathFilenameIn(&sb, path);
	return str::NewFromBuffer(sb.buffer);
}

void FilepathFilenameNoExtIn(str::Builder *sb, str::String path)
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

str::String FilepathFilenameNoExt(str::String path)
{
	auto sb = str::Builder{};
	FilepathFilenameNoExtIn(&sb, path);
	return str::NewFromBuffer(sb.buffer);
}

// FilepathExtension returns the file extension including the dot.
void FilepathExtensionIn(str::Builder *sb, str::String path)
{
	auto dot = path.FindLast('.');
	if (dot < 0)
	{
		return;
	}
	return sb->Append(path.ToView(dot, path.Length()));
}

str::String FilepathExtension(str::String path)
{
	auto sb = str::Builder{};
	FilepathExtensionIn(&sb, path);
	return str::NewFromBuffer(sb.buffer);
}

// SetFilepathExtension replaces the portion of the path after the last dot character with the supplied extension.
// If no dot character is found, a dot character and the supplied extension are appended to the path.
void SetFilepathExtensionIn(str::Builder *path, str::String ext)
{
	auto dot = path->FindLast('.');
	if (dot < 0)
	{
		path->Append(".");
		path->Append(ext);
		return;
	}
	path->Resize(dot + ext.Length());
	arr::Copy(ext.buffer, path->ToView(dot, path->Length()).buffer);
}

str::String SetFilepathExtension(str::String path, str::String ext)
{
	auto sb = str::NewBuilderWithCapacity(path.Length());
	sb.Append(path);
	SetFilepathExtensionIn(&sb, ext);
	return str::NewFromBuffer(sb.buffer);
}
#endif

}
