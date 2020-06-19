#pragma once

#if defined(__linux__)
	#include "Linux/Process.h"
#else
	#error Unsupported platform.
#endif

struct String;

#define Abort(format, ...) AbortActual(format, __FILE__, __func__, __LINE__, ##__VA_ARGS__)
void AbortActual(String format, String fileName, String functionName, s64 lineNumber, ...);
void AbortActual(const char *format, const char *fileName, const char *functionName, s64 lineNumber, ...);
