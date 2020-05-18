#include <sys/mman.h>

// ucontext_t is the recommended method for implementing fibers on Linux, but it is apparently very
// slow because it preserves each fiber's signal mask.
//
// Our method (from http://www.1024cores.net/home/lock-free-algorithms/tricks/fibers) still creates
// ucontext_t contexts, but it uses _setjmp/_longjmp to switch between them, thus avoiding syscalls
// for saving and setting signal masks.  It works by swapping to the fiber's context during fiber
// creation, setting up a long jump into the new fiber's context using _setjmp, then swapping back
// to the calling context.  Now when we want to switch to the fiber we can _longjmp directly into
// the fiber's context. 

THREAD_LOCAL struct ThreadLocalFibersContext
{
	Fiber threadFiber;
	Fiber *activeFiber;
} threadLocalFibersContext;

struct FibersContext
{
	char *fiberStackMemory;
	volatile s64 fiberCount;
	s64 pageSize;
} fibersContext;

struct FiberCreationInfo
{
	FiberProcedure procedure;
	void *parameter;
	jmp_buf *jumpBuffer;
	ucontext_t *callingContext;
};

void RunFiber(void *fiberCreationInfoPointer)
{
	auto fiberCreationInfo = (FiberCreationInfo *)fiberCreationInfoPointer;
	auto procedure = fiberCreationInfo->procedure;
	auto parameter = fiberCreationInfo->parameter;
	if (!_setjmp(*fiberCreationInfo->jumpBuffer))
	{
		ucontext_t context = {};
		swapcontext(&context, fiberCreationInfo->callingContext);
	}
	procedure(parameter);
	pthread_exit(NULL);
}

#define FIBER_STACK_SIZE 81920

void CreateFiber(Fiber *fiber, FiberProcedure procedure, void *parameter)
{
	getcontext(&fiber->context);
	auto fiberIndex = AtomicFetchAndAdd(&fibersContext.fiberCount, 1);
	fiber->context.uc_stack.ss_sp = fibersContext.fiberStackMemory + ((fiberIndex * FIBER_STACK_SIZE) + ((fiberIndex + 1) * fibersContext.pageSize));
	fiber->context.uc_stack.ss_size = FIBER_STACK_SIZE;
	fiber->context.uc_link = 0;
	ucontext_t temporaryContext;
	FiberCreationInfo fiberCreationInfo =
	{
		.procedure = procedure,
		.parameter = parameter,
		.jumpBuffer = &fiber->jumpBuffer,
		.callingContext = &temporaryContext,
	};
	makecontext(&fiber->context, (void(*)())RunFiber, 1, &fiberCreationInfo);
	swapcontext(&temporaryContext, &fiber->context);
}

void ConvertThreadToFiber(Fiber *fiber)
{
	threadLocalFibersContext.activeFiber = fiber;
}

// @TODO: Prevent two fibers from running at the same time.
void SwitchToFiber(Fiber *fiber)
{
	if (!_setjmp(threadLocalFibersContext.activeFiber->jumpBuffer))
	{
		threadLocalFibersContext.activeFiber = fiber;
		_longjmp(fiber->jumpBuffer, 1);
	}
}

Fiber *GetCurrentFiber()
{
	return threadLocalFibersContext.activeFiber;
}

void InitializeFibers(s64 maxFiberCount)
{
	fibersContext.pageSize = GetPageSize();
	fibersContext.fiberCount = 0;
	fibersContext.fiberStackMemory = (char *)AllocateMemory((maxFiberCount * FIBER_STACK_SIZE) + ((maxFiberCount + 1) * fibersContext.pageSize));
	// Protect the memory surrounding each fiber's stack to detect writing off out of bounds.
	// @TODO: Disable this in release mode to save memory?
	for (auto i = 0; i <= maxFiberCount; i++)
	{
		mprotect(fibersContext.fiberStackMemory + ((i * FIBER_STACK_SIZE) + (i * fibersContext.pageSize)), fibersContext.pageSize, PROT_NONE);
	}
}
