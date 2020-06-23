#pragma once

#if defined(__linux__)
	#include "Linux/Log.h"
#else
	#error Unsupported platform.
#endif

#include "PCH.h"

#include "Code/Common.h"

enum LogLevel
{
	LogLevelVerbose,
	LogLevelInfo,
	LogLevelError,
	LogLevelAbort,
};

struct String;

void InitializeLog();

void ConsolePrint(String fmt, ...);
void ConsolePrint(const char *fmt, ...);

#define LogPrint(lvl, cat, fmt, ...) LogPrintActual(__FILE__, __func__, __LINE__, lvl, cat, fmt, ##__VA_ARGS__);
void LogPrintActual(String file, String func, s64 line, LogLevel l, String fmt, ...);
void LogPrintActual(const char *file, const char *func, s64 line, LogLevel l, String category, const char *fmt, ...);
void LogPrintVarArgs(String file, String func, s64 line, LogLevel l, String fmt, String category, va_list args);

void SetLogLevel(LogLevel l);
