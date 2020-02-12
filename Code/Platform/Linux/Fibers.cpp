#include <sys/mman.h>

#include "Platform/Fibers.h"
#include "Platform/Memory.h"

// ucontext_t is the recommended method for implementing fibers on Linux, but it is apparently very
// slow because it preserves each fiber's signal mask.
//
// Our method (from http://www.1024cores.net/home/lock-free-algorithms/tricks/fibers) still creates
// ucontext_t contexts, but it uses _setjmp/_longjmp to switch between them, thus avoiding syscalls
// for saving and setting signal masks.  It works by swapping to the fiber's context during fiber
// creation, setting up a long jump into the new fiber's context using _setjmp, then swapping back
// to the calling context.  Now when we want to switch to the fiber we can _longjmp directly into
// the fiber's context. 

struct
{
	char *fiberStackMemory;
	volatile s32 fiberCount;
	size_t pageSize;
} linuxFibersContext;

__thread struct
{
	PlatformFiber threadFiber;
	PlatformFiber *activeFiber;
} threadLocalLinuxFibersContext;

struct PlatformFiberCreationInfo
{
	PlatformFiberProcedure procedure;
	void *parameter;
	jmp_buf *jumpBuffer;
	ucontext_t *callingContext;
};

void PlatformRunFiber(void *fiberCreationInfoPointer)
{
	PlatformFiberCreationInfo *fiberCreationInfo = (PlatformFiberCreationInfo *)fiberCreationInfoPointer;
	PlatformFiberProcedure procedure = fiberCreationInfo->procedure;
	void *parameter = fiberCreationInfo->parameter;
	if (!_setjmp(*fiberCreationInfo->jumpBuffer))
	{
		ucontext_t context = {};
		swapcontext(&context, fiberCreationInfo->callingContext);
	}
	procedure(parameter);
	pthread_exit(NULL);
}

#define FIBER_STACK_SIZE 81920

// @TODO: Rather than mprotecting memory, we should just check to see if the stack pointer is out-of-bound.
void PlatformCreateFiber(PlatformFiber *fiber, PlatformFiberProcedure procedure, void *parameter)
{
	getcontext(&fiber->context);
	s32 fiberIndex = PlatformAtomicFetchAndAddS32(&linuxFibersContext.fiberCount, 1);
	fiber->context.uc_stack.ss_sp = linuxFibersContext.fiberStackMemory + ((fiberIndex * FIBER_STACK_SIZE) + ((fiberIndex + 1) * linuxFibersContext.pageSize));
	fiber->context.uc_stack.ss_size = FIBER_STACK_SIZE;
	fiber->context.uc_link = 0;
	ucontext_t temporaryContext;
	PlatformFiberCreationInfo fiberCreationInfo = {
		.procedure = procedure,
		.parameter = parameter,
		.jumpBuffer = &fiber->jumpBuffer,
		.callingContext = &temporaryContext,
	};
	makecontext(&fiber->context, (void(*)())PlatformRunFiber, 1, &fiberCreationInfo);
	swapcontext(&temporaryContext, &fiber->context);
}

void PlatformConvertThreadToFiber(PlatformFiber *fiber)
{
	threadLocalLinuxFibersContext.activeFiber = fiber;
}

// @TODO: Prevent two fibers from running at the same time.
void PlatformSwitchToFiber(PlatformFiber *fiber)
{
	if (!_setjmp(threadLocalLinuxFibersContext.activeFiber->jumpBuffer))
	{
		threadLocalLinuxFibersContext.activeFiber = fiber;
		_longjmp(fiber->jumpBuffer, 1);
	}
}

PlatformFiber *PlatformGetCurrentFiber()
{
	return threadLocalLinuxFibersContext.activeFiber;
}

void PlatformInitializeFibers(u32 fiberCount)
{
	linuxFibersContext.pageSize = PlatformGetPageSize();
	linuxFibersContext.fiberStackMemory = (char *)PlatformAllocateMemory((fiberCount * FIBER_STACK_SIZE) + ((fiberCount + 1) * linuxFibersContext.pageSize));
	for (s32 i = 0; i <= fiberCount; i++)
	{
		mprotect(linuxFibersContext.fiberStackMemory + ((i * FIBER_STACK_SIZE) + (i * linuxFibersContext.pageSize)), linuxFibersContext.pageSize, PROT_NONE);
	}
}
