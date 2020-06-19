#include "Process.h"
#include "String.h"
#include "Log.h"

void DoAbortActual(String format, String fileName, String functionName, s64 lineNumber, va_list arguments)
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

void AbortActual(String format, String fileName, String functionName, s64 lineNumber, ...)
{
	va_list arguments;
	va_start(arguments, lineNumber);
	DoAbortActual(format, fileName, functionName, lineNumber, arguments);
}

void AbortActual(const char *format, const char *fileName, const char *functionName, s64 lineNumber, ...)
{
	va_list arguments;
	va_start(arguments, lineNumber);
	DoAbortActual(format, fileName, functionName, lineNumber, arguments);
}
