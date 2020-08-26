#pragma once

#include "../PCH.h"
#include "Common.h"

#if __x86_64__
	#define CPUHintSpinWaitLoop() _mm_pause()
	#define CPUCacheLineSize 64
#else
	#error Unsupported CPU type.
#endif

s64 CPUProcessorCount();
s64 CPUPageSize();
