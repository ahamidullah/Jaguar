#pragma once

#include "../Array.h"
#include "../PCH.h"
#include "Common.h"

struct Fiber
{
	ucontext_t context;
	jmp_buf jumpBuffer;
	Allocator *baseContextAllocator;
	Array<Allocator *> contextAllocatorStack;
	Allocator *contextAllocator;
	#ifdef ThreadSanitizerBuild
		void *tsan;
	#endif

	void Switch();
	void Delete();
};

void InitializeFibers();
typedef void (*FiberProcedure)(void *);
Fiber *NewFiber(FiberProcedure proc, void *param);
Fiber *ConvertThreadToFiber();
Fiber *RunningFiber();
