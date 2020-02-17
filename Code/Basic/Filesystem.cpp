// Returns all but the last component of the path.
String GetFilepathDirectory(const String &path)
{
	auto slashIndex = FindLastIndex(path, '/');
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
	auto slashIndex = FindLastIndex(path, '/');
	if (slashIndex < 0)
	{
		return "";
	}
	auto filenameLength = Length(path) - (slashIndex + 1);
	auto filename = CreateString(filenameLength);
	CopyMemory(&path[0] + slashIndex + 1, &filename[0], filenameLength);
	return filename;
}

// Returns the file extension including the dot.
String GetFilepathExtension(const String &path)
{
	auto dotIndex = FindLastIndex(path, '.');
	if (dotIndex < 0)
	{
		return "";
	}
	auto fileExtensionLength = Length(path) - (dotIndex + 1);
	auto fileExtension = CreateString(fileExtensionLength);
	CopyMemory(&path[0] + (dotIndex + 1), &fileExtension[0], fileExtensionLength);
	return fileExtension;
}

bool SetFilepathExtension(String *path, const String &newExtension)
{
	auto dotIndex = FindLastIndex(*path, '.');
	if (dotIndex < 0)
	{
		return false;
	}
	Resize(path, dotIndex + 1 + Length(newExtension));
	CopyMemory(&newExtension[0], &(*path)[dotIndex + 1], Length(newExtension));
	return true;
}

// Concatenates two strings and inserts a '/' between them.
String JoinFilepaths(const String &a, const String &b)
{
	auto result = CreateString(Length(a) + Length(b) + 1);
	CopyMemory(&a[0], &result[0], Length(a));
	result[Length(a)] = '/';
	CopyMemory(&b[0], &result[0] + Length(a) + 1, Length(b));
	return result;
}
