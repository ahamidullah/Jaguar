#include "Basic.h"

void ConsolePrintVarArgs(String format, va_list arguments)
{
	WriteToConsole(FormatStringVarArgs(format, arguments));
}

void ConsolePrint(const String &format, ...)
{
	va_list arguments;
	va_start(arguments, format);
	ConsolePrintVarArgs(format, arguments);
	va_end(arguments);
}

void ConsolePrint(const char *format, ...)
{
	va_list arguments;
	va_start(arguments, format);
	ConsolePrintVarArgs(format, arguments);
	va_end(arguments);
}

void LogPrintVarArgs(LogType t, String format, va_list arguments)
{
	ConsolePrintVarArgs(format, arguments);
	// @TODO: Print message to a log file.
}

// @TODO: Handle logType.
void LogPrint(LogType t, String format, ...)
{
	va_list arguments;
	va_start(arguments, format);
	LogPrintVarArgs(t, format, arguments);
	va_end(arguments);
}

void LogPrint(LogType t, const char *format, ...)
{
	va_list arguments;
	va_start(arguments, format);
	LogPrintVarArgs(t, format, arguments);
	va_end(arguments);
}
