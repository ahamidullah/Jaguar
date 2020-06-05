#include "Log.h"
#include "Process.h"

void ConsolePrintVarArgs(const String &format, va_list arguments)
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

void LogPrintVarArgs(LogType logType, const String &format, va_list arguments)
{
	ConsolePrintVarArgs(format, arguments);
	// @TODO: Print message to a log file.
}

// @TODO: Handle logType.
void LogPrint(LogType logType, const String &format, ...)
{
	va_list arguments;
	va_start(arguments, format);
	LogPrintVarArgs(logType, format, arguments);
	va_end(arguments);
}

void LogPrint(LogType logType, const char *format, ...)
{
	va_list arguments;
	va_start(arguments, format);
	LogPrintVarArgs(logType, format, arguments);
	va_end(arguments);
}

void DoAbortActual(const String &format, const String &fileName, const String &functionName, s32 lineNumber, va_list arguments)
{
	LogPrint(ERROR_LOG, "###########################################################################\n");
	LogPrint(ERROR_LOG, "[PROGRAM ABORT]\n");
	LogPrintVarArgs(ERROR_LOG, format, arguments);
	LogPrint(ERROR_LOG, "\n");
	PrintStacktrace();
	LogPrint(ERROR_LOG, "###########################################################################\n");
	va_end(arguments);
	SignalDebugBreakpoint();
	ExitProcess(PROCESS_EXIT_FAILURE);
}

void AbortActual(const String &format, const String &fileName, const String &functionName, s32 lineNumber, ...)
{
	va_list arguments;
	va_start(arguments, lineNumber);
	DoAbortActual(format, fileName, functionName, lineNumber, arguments);
}

void AbortActual(const char *format, const char *fileName, const char *functionName, s32 lineNumber, ...)
{
	va_list arguments;
	va_start(arguments, lineNumber);
	DoAbortActual(format, fileName, functionName, lineNumber, arguments);
}
