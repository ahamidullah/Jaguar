#include "Basic.h"

String ReadEntireFile(String path, bool *error)
{
	auto f = OpenFile(path, OPEN_FILE_READ_ONLY, error);
	if (*error)
	{
		return "";
	}
	Defer(CloseFile(f));
	auto length = GetFileLength(f, error);
	if (*error)
	{
		return "";
	}
	auto result = ReadFromFile(f, length, error);
	if (*error)
	{
		return "";
	}
	*error = false;
	return result;
}

PlatformTime GetFilepathLastModifiedTime(String path, bool *error)
{
	auto f = OpenFile(path, OPEN_FILE_READ_ONLY, error);
	if (*error)
	{
		return PlatformTime{};
	}
	Defer(CloseFile(f));
	auto result = GetFileLastModifiedTime(f, error);
	if (*error)
	{
		return PlatformTime{};
	}
	*error = false;
	return result;
}
