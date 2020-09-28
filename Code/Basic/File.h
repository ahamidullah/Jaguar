#pragma once

#if __linux__
	#include "Linux/File.h"
#else
	#error Unsupported platform.
#endif

void ReadEntireFileIn(StringBuilder *sb, String path, bool *err);
String ReadEntireFile(String path, bool *err);
