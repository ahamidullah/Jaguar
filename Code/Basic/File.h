#pragma once

#if __linux__
	#include "Linux/File.h"
#else
	#error Unsupported platform.
#endif

bool ReadEntireFileIn(StringBuilder *sb, String path);
String ReadEntireFile(String path, bool *err);
