#include "Basic.h"
#include "Memory.h"
#include "Fiber.h"

void InitializeBasic()
{
	InitializeMemory();
	InitializeFibers();
}
