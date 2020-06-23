#pragma once

#if defined(__linux__)
	#include "Linux/Process.h"
#else
	#error Unsupported platform.
#endif

struct String;

#define Abort(fmt, ...) AbortActual(__FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
void AbortActual(String file, String func, s64 line, String fmt, ...);
void AbortActual(const char *file, const char *func, s64 line, const char *fmt, ...);
