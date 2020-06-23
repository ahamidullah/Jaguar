#include "Assert.h"
#include "Log.h"
#include "Process.h"

void AssertActual(bool test, const char *file, const char *func, s64 line, const char *src)
{
	#if DEBUG_BUILD
		if (!test)
		{
			LogPrint(LogLevelError, "Abort", "%s: %s: line %d: assertion failed '%s'.\n", file, func, line, src);
			PrintStacktrace();
			SignalDebugBreakpoint();
			ExitProcess(PROCESS_EXIT_FAILURE);
		}
	#endif
}
