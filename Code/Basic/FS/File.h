#pragma once

#if __linux__
	#include "Linux/File.h"
#else
	#error Unsupported platform.
#endif

namespace filesystem
{

void ReadEntireFileIn(arr::array<u8> *a, str::String path, bool *err);
arr::array<u8> ReadEntireFile(str::String path, bool *err);

}
