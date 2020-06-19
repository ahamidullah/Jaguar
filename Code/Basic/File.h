#pragma once

#if defined(__linux__)
	#include "Linux/File.h"
#else
	#error Unsupported platform.
#endif

String ReadEntireFile(String path, bool *error);
PlatformTime GetFilepathLastModifiedTime(String path, bool *error);
