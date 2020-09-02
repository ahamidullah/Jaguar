#include "Basic.h"
#include "Memory.h"
#include "Fiber.h"
#include "Log.h"

auto isBasicInitialized = false;

void InitializeBasic()
{
	InitializeLog();
	InitializeFibers();
	isBasicInitialized = true;
}
