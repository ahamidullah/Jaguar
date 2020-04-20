#pragma once

#include <ucontext.h>
#include <setjmp.h>

struct Fiber
{
	ucontext_t context;
	jmp_buf jumpBuffer;
};

typedef void (*FiberProcedure)(void *);

void InitializeFibers(u32 maxFiberCount);
void CreateFiber(Fiber *fiber, FiberProcedure procedure, void *parameter);
void ConvertThreadToFiber(Fiber *fiber);
void SwitchToFiber(Fiber *fiber);
Fiber *GetCurrentFiber();
