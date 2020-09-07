#include "Basic.h"
#include "Memory.h"
#include "Fiber.h"
#include "Log.h"
#include "Thread.h"

auto isBasicInitialized = false;

void InitializeBasic()
{
	if (ThreadCount() != 1)
	{
		// We require single-threading when running initialization.
		Abort("Basic", "InitializeBasic was called by a multi-threaded process (call InitializeBasic before spawning any threads).");
	}
	InitializeMemory();
	InitializeLog();
	InitializeFibers();
	isBasicInitialized = true;
}
