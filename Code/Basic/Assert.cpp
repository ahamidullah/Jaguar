#include "Assert.h"
#include "Log.h"
#include "Process.h"

void AssertActual(bool test, const char *file, const char *func, s64 line, const char *src)
{
	#if DebugBuild
		if (!test)
		{
			Abort("Assert", "%s: %s: line %d: assertion failed '%s'.", file, func, line, src);
		}
	#endif
}
