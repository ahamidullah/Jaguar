#pragma once

#if defined(__linux__)
	#include "Linux/Log.h"
#else
	#error Unsupported platform.
#endif
#include "String.h"
#include "PCH.h"

#include "Code/Common.h"

enum LogType
{
	INFO_LOG,
	ERROR_LOG,
	ABORT_LOG,
};

#define Abort(format, ...) AbortActual(format, __FILE__, __func__, __LINE__, ##__VA_ARGS__)
void AbortActual(const String &format, const String &fileName, const String &functionName, s32 lineNumber, ...);

void ConsolePrintVarargs(const String &format, va_list argumentList);
void ConsolePrint(const String &format, ...);

void LogPrint(LogType logType, const String &format, ...);
void LogPrintVarArgs(LogType logType, const String &format, va_list arguments);
