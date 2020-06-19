#pragma once

#if defined(__linux__)
	#include "Linux/Log.h"
#else
	#error Unsupported platform.
#endif

#include "PCH.h"

#include "Code/Common.h"

enum LogType
{
	INFO_LOG,
	ERROR_LOG,
	ABORT_LOG,
};

struct String;

void ConsolePrint(String format, ...);
void ConsolePrint(const char *format, ...);

void LogPrint(LogType t, String format, ...);
void LogPrint(LogType t, const char *format, ...);
void LogPrintVarArgs(LogType t, String format, va_list arguments);
