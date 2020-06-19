#pragma once

#include "../PCH.h"

#include "Code/Common.h"

struct Fiber
{
	ucontext_t context;
	jmp_buf jumpBuffer;
	#if defined(THREAD_SANITIZER_BUILD)
		void *tsanFiber;
	#endif
};

typedef void (*FiberProcedure)(void *);

void InitializeFibers();
void CreateFiber(Fiber *fiber, FiberProcedure procedure, void *parameter);
void ConvertThreadToFiber(Fiber *fiber);
void SwitchToFiber(Fiber *fiber);
