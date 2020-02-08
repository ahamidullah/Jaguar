#pragma once

enum LogType {
	INFO_LOG,
	ERROR_LOG,
	ABORT_LOG,
};

void ConsolePrintVarargs(const char *format, va_list argument_list);
void ConsolePrint(const char *format, ...);
void LogPrint(LogType logType, const char *format, ...);

