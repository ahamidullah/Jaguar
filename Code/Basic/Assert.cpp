#include "Assert.h"
#include "Log.h"
#include "Process.h"

void AssertActual(bool test, const char *file, const char *func, s64 line, const char *src)
{
	#ifdef DebugBuild
		if (!test)
		{
			Abort("Global", "%s: %s: line %d: assertion failed '%s'.\n", file, func, line, src);
		}
	#endif
}
