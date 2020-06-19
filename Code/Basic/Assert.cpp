#include "Assert.h"
#include "Log.h"
#include "Process.h"

void AssertActual(bool test, const char *fileName, const char *functionName, s64 lineNumber, const char *testName)
{
	#if defined(DEBUG_BUILD)
		if (!test)
		{
			LogPrint(ERROR_LOG, "%s: %s: line %d: assertion failed '%s'.\n", fileName, functionName, lineNumber, testName);
			PrintStacktrace();
			SignalDebugBreakpoint();
			ExitProcess(PROCESS_EXIT_FAILURE);
		}
	#endif
}
