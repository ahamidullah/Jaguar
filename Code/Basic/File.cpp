ReadFileResult ReadEntireFile(const String &path)
{
	auto [file, error] = OpenFile(path, OPEN_FILE_READ_ONLY);
	if (error)
	{
		return ReadFileResult{.error = true};
	}
	Defer(CloseFile(file));
	auto fileLength = GetFileLength(file);
	return ReadFile(file, fileLength);
}
