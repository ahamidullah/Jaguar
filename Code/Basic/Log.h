#pragma once

enum LogType
{
	INFO_LOG,
	ERROR_LOG,
	ABORT_LOG,
};

void ConsolePrintVarargs(const String &format, va_list argumentList);
void ConsolePrint(const String &format, ...);
void LogPrint(LogType logType, const String &format, ...);
