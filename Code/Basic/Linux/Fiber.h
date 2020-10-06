#pragma once

#include "../Array.h"
#include "../PCH.h"
#include "Common.h"

	struct PlatformContext
	{
		void *rip, *rsp;
		void *rbx, *rbp, *r12, *r13, *r14, *r15;
	};


#define NEW_FIBER 1

typedef void (*FiberProcedure)(void *);

struct RunFiberParameters
{
	FiberProcedure procedure;
	void *parameter;
};

struct Fiber
{
#if !NEW_FIBER
	ucontext_t context;
	jmp_buf jumpBuffer;
	Array<Allocator *> contextAllocatorStack;
	Allocator *contextAllocator;
#else
	PlatformContext context;
	RunFiberParameters runParameters;
	Array<Allocator *> contextAllocatorStack;
	Allocator *contextAllocator;
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
