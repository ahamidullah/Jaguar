#pragma once

#if __linux__
	#include "Linux/File.h"
#else
	#error Unsupported platform.
#endif

void ReadEntireFileIn(array::Array<u8> *a, String path, bool *err);
array::Array<u8> ReadEntireFile(String path, bool *err);
