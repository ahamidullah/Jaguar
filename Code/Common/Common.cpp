#include "Platform/Platform.h"
#include "Common.h"
#include "Log.h"

void Abort(const char *format, ...) {
	// @TODO: Print message to a log file as well.
	if (debug) {
		va_list arguments;
		va_start(arguments, format);
		ConsolePrint("###########################################################################\n");
		ConsolePrint("[PROGRAM ABORT]\n");
		ConsolePrintVarargs(format, arguments);
		PlatformPrintStacktrace();
		ConsolePrint("###########################################################################\n");
		va_end(arguments);
		PlatformSignalDebugBreakpoint();
	}
	PlatformExitProcess(PLATFORM_FAILURE_EXIT_CODE);
}

void AssertActual(bool test, const char *fileName, const char *functionName, s32 lineNumber, const char *testName) {
	if (debug && !test) {
		ConsolePrint("%s: %s: line %d: assertion failed '%s'\n", fileName, functionName, lineNumber, testName);
		PlatformPrintStacktrace();
		PlatformSignalDebugBreakpoint();
		PlatformExitProcess(1);
	}
}
