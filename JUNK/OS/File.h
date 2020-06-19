#pragma once

#if defined(__linux__)
	#include "Linux/File.h"
#else
	#error Unsupported platform.
#endif
#include "Time.h"

struct String;

String ReadEntireFile(const String &path, bool *error);
PlatformTime GetFilepathLastModifiedTime(const String &filepath, bool *error);
