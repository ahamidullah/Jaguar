#pragma once

#if __linux__
	#include "Linux/Log.h"
#else
	#error Unsupported platform.
#endif
#include "PCH.h"
#include "Common.h"

enum LogLevel
{
	VerboseLog,
	InfoLog,
	ErrorLog,
	FatalLog,
};

struct String;
void SetLogLevel(LogLevel l);
void ConsolePrint(String fmt, ...);
//void ConsolePrint(const char *fmt, ...);
#define LogPrint(lvl, cat, fmt, ...) LogPrintActual(__FILE__, __func__, __LINE__, lvl, cat, fmt, ##__VA_ARGS__)
void LogPrintActual(String file, String func, s64 line, LogLevel l, String category, String fmt, ...);
void LogPrintActual(const char *file, const char *func, s64 line, LogLevel l, const char *category, const char *fmt, ...);
#define LogVerbose(cat, fmt, ...) LogPrint(VerboseLog, cat, fmt, ##__VA_ARGS__)
#define LogInfo(cat, fmt, ...) LogPrint(InfoLog, cat, fmt, ##__VA_ARGS__)
#define LogError(cat, fmt, ...) LogPrint(ErrorLog, cat, fmt, ##__VA_ARGS__)
#define LogFatal(cat, fmt, ...) LogPrint(FatalLog, cat, fmt, ##__VA_ARGS__)
// @TODO: Move this to its own file (or Process.cpp?).
#define Abort(category, fmt, ...) AbortActual(__FILE__, __func__, __LINE__, category, fmt, ##__VA_ARGS__)
void AbortActual(String file, String func, s64 line, String category, String fmt, ...);
void AbortActual(const char *file, const char *func, s64 line, const char *category, const char *fmt, ...);
