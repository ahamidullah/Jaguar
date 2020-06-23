#include "Process.h"
#include "String.h"
#include "Log.h"

void DoAbortActual(String file, String func, s64 line, String fmt, va_list args)
{
	LogPrint(LogLevelAbort, "Abort", "###########################################################################\n");
	LogPrint(LogLevelAbort, "Abort", "[PROGRAM ABORT]\n");
	LogPrintVarArgs(file, func, line, LogLevelAbort, "Abort", fmt, args);
	LogPrint(LogLevelAbort, "Abort", "\n");
	PrintStacktrace();
	LogPrint(LogLevelAbort, "Abort", "###########################################################################\n");
	va_end(args);
	SignalDebugBreakpoint();
	ExitProcess(PROCESS_EXIT_FAILURE);
}

void AbortActual(String file, String func, s64 line, String fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	DoAbortActual(file, func, line, fmt, args);
}

void AbortActual(const char *file, const char *func, s64 line, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	DoAbortActual(file, func, line, fmt, args);
}
