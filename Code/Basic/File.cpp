String ReadEntireFile(const String &path, bool *error)
{
	auto file = OpenFile(path, OPEN_FILE_READ_ONLY, error);
	if (*error)
	{
		return "";
	}
	Defer(CloseFile(file));
	auto fileLength = GetFileLength(file, error);
	if (*error)
	{
		return "";
	}
	auto result = ReadFromFile(file, fileLength, error);
	if (*error)
	{
		return "";
	}
	*error = false;
	return result;
}

PlatformTime GetFilepathLastModifiedTime(const String &filepath, bool *error)
{
	auto sourceFile = OpenFile(filepath, OPEN_FILE_READ_ONLY, error);
	if (*error)
	{
		return PlatformTime{};
	}
	Defer(CloseFile(sourceFile));
	auto result = GetFileLastModifiedTime(sourceFile, error);
	if (*error)
	{
		return PlatformTime{};
	}
	*error = false;
	return result;
}
