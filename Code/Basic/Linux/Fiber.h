#pragma once

#include "../PCH.h"

#include "Code/Common.h"

struct Fiber
{
	ucontext_t context;
	jmp_buf jumpBuffer;
};

typedef void (*FiberProcedure)(void *);

void InitializeFibers();
void CreateFiber(Fiber *fiber, FiberProcedure procedure, void *parameter);
void ConvertThreadToFiber(Fiber *fiber);
void SwitchToFiber(Fiber *fiber);
Fiber *GetCurrentFiber();
