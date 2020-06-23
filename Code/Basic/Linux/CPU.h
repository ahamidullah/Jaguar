#pragma once

#include "../PCH.h"

#include "Code/Common.h"

#if __x86_64__
	#define CPUHintSpinWaitLoop() _mm_pause()
	#define CPU_CACHE_LINE_SIZE 64
#else
	#error Unsupported CPU type.
#endif

s64 CPUProcessorCount();
s64 CPUPageSize();
