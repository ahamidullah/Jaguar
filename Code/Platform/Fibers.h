#pragma once

#if defined(__linux__)
	#include "Platform/Linux/Fibers.h"
#else
	#error unsupported platform
#endif

void PlatformCreateFiber(PlatformFiber *fiber, PlatformFiberProcedure procedure, void *parameter);
void PlatformSwitchToFiber(PlatformFiber *fiber);
void PlatformConvertThreadToFiber(PlatformFiber *fiber);
PlatformFiber *PlatformGetCurrentFiber();
