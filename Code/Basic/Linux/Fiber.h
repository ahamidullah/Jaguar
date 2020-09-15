#pragma once

#include "../Array.h"
#include "../PCH.h"
#include "Common.h"

struct Fiber
{
	ucontext_t context;
	jmp_buf jumpBuffer;
	Array<Allocator *> contextAllocatorStack;
	Allocator *contextAllocator;
	#ifdef ThreadSanitizerBuild
		void *tsan;
	#endif

	void Switch();
	void Delete();
};

typedef void (*FiberProcedure)(void *);
Fiber NewFiber(FiberProcedure proc, void *param);
void ConvertThreadToFiber(Fiber *f);
Fiber *RunningFiber();
