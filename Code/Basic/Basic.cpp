#include "PCH.h"
#include "Log.h"
#include "Process.h"
#include "Fiber.h"
#include "Memory.h"

#include "Code/Common.h"

void InitializeBasic()
{
	InitializeMemory();
	InitializeFibers();
}
