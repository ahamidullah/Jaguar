#pragma once

#if defined(__linux__)
	#include "Platform/Linux/Time.h"
#else
	#error unsupported platform
#endif

PlatformTime PlatformGetCurrentTime();
f64 PlatformTimeDifference(PlatformTime start, PlatformTime end);
void PlatformSleep(u32 milliseconds);
