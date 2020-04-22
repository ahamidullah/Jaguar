#include "Basic.h"

#if defined(__linux__)
	#include "Linux/Start.cpp"
	#include "Linux/Memory.cpp"
	#include "Linux/Process.cpp"
	#include "Linux/File.cpp"
	#include "Linux/Log.cpp"
	#include "Linux/Thread.cpp"
	#include "Linux/Mutex.cpp"
	#include "Linux/Semaphore.cpp"
	#include "Linux/Atomic.cpp"
	#include "Linux/Fiber.cpp"
	#include "Linux/Time.cpp"
	#include "Linux/DLL.cpp"
#else
	#error unsupported platform
#endif

#include "Memory.cpp"
#include "File.cpp"
#include "Log.cpp"
#include "String.cpp"
#include "Filesystem.cpp"
#include "Parser.cpp"

void AssertActual(bool test, const String &fileName, const String &functionName, s32 lineNumber, const String &testName)
{
	if (debug && !test)
	{
		LogPrint(ERROR_LOG, "%k: %k: line %d: assertion failed '%k'.\n", fileName, functionName, lineNumber, testName);
		PrintStacktrace();
		SignalDebugBreakpoint();
		ExitProcess(PROCESS_EXIT_FAILURE);
	}
}

void AbortActual(const String &format, const String &fileName, const String &functionName, s32 lineNumber, ...)
{
	va_list arguments;
	va_start(arguments, lineNumber);
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

void InitializeBasic(s64 maxFiberCount)
{
	InitializeFibers(maxFiberCount);
}
