#pragma once

#include "../Array.h"
#include "../PCH.h"
#include "Common.h"

	struct SystemContext
	{
		void *rip, *rsp;
		void *rbx, *rbp, *r12, *r13, *r14, *r15;
	};

#define NEW_FIBER 1

#if !NEW_FIBER
#include <setjmp.h>
#endif

typedef void (*FiberProcedure)(void *);

struct Fiber
{
#if !NEW_FIBER
	ucontext_t context;
	jmp_buf jumpBuffer;
	Allocator *contextAllocator;
	Array<Allocator *> contextAllocatorStack;
#else
	SystemContext context;
	Allocator *contextAllocator;
	Array<Allocator *> contextAllocatorStack;
#endif
	#ifdef ThreadSanitizerBuild
		void *tsan;
	#endif

	void Switch();
	void Delete();
};

Fiber NewFiber(FiberProcedure proc, void *param);
void ConvertThreadToFiber(Fiber *f);
Fiber *RunningFiber();
