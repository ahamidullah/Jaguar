#include "Basic.h"
#include "Memory.h"
#include "Fiber.h"
#include "Log.h"

void InitializeBasic()
{
	InitializeLog();
	InitializeFibers();
}
