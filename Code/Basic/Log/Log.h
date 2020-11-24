#pragma once

#if __linux__
	#include "Log_Linux.h"
#else
	#error Unsupported platform.
#endif
#include "Basic/String.h"
#include "Basic/PCH.h"
#include "Common.h"

namespace log
{

enum class Level
{
	Verbose,
	Info,
	Error,
	Fatal,
};

void SetLevel(Level l);
void Console(str::String fmt, ...);
#define Print(lvl, cat, fmt, ...) PrintActual(__FILE__, __func__, __LINE__, lvl, cat, fmt, ##__VA_ARGS__)
void PrintActual(str::String file, str::String func, s64 line, Level l, str::String cat, str::String fmt, ...);
void PrintActual(const char *file, const char *func, s64 line, Level l, const char *cat, const char *fmt, ...);
#define Verbose(cat, fmt, ...) Print(Level::Verbose, cat, fmt, ##__VA_ARGS__)
#define Info(cat, fmt, ...) Print(Level::Info, cat, fmt, ##__VA_ARGS__)
#define Error(cat, fmt, ...) Print(Level::Error, cat, fmt, ##__VA_ARGS__)
#define Fatal(cat, fmt, ...) Print(Level::Fatal, cat, fmt, ##__VA_ARGS__)
#define Abort(cat, fmt, ...) AbortActual(__FILE__, __func__, __LINE__, cat, fmt, ##__VA_ARGS__)
void AbortActual(str::String file, str::String func, s64 line, str::String cat, str::String fmt, ...);
void AbortActual(const char *file, const char *func, s64 line, const char *cat, const char *fmt, ...);

}
