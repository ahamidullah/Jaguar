#include "Assert.h"
#include "Log.h"

void AssertActual(bool test, const char *file, const char *func, s64 line, const char *src) {
	#ifdef DebugBuild
		if (!test) {
			log::Abort("Assert", "%s: %s: line %d: assertion failed '%s'.", file, func, line, src);
		}
	#endif
}
