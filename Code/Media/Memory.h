#pragma once

#if defined(__linux__)
	#include "Platform/Linux/Memory.h"
#else
	#error unsupported platform
#endif

void *PlatformAllocateMemory(size_t size);
void PlatformFreeMemory(void *memory, size_t size);
size_t PlatformGetPageSize();
void PlatformPrintStacktrace();
