#include "../Fiber.h"
#include "../Thread.h"
#include "../Atomic.h"
#include "../Memory.h"

// ucontext_t is the recommended method for implementing fibers on Linux, but it is apparently very
// slow because it preserves each fiber's signal mask.
//
// Our method (from http://www.1024cores.net/home/lock-free-algorithms/tricks/fibers) still creates
// a ucontext_t, but it uses _setjmp/_longjmp to switch between them, thus avoiding syscalls for
// saving and setting signal masks.  It works by swapping to the fiber's context during fiber
// creation, setting up a long jump into the new fiber's context using _setjmp, then swapping back
// to the calling context.  Now, when we want to switch to the fiber, we can _longjmp directly into
// the fiber's context. 

// @TODO: Remove all THREAD_LOCAL.
THREAD_LOCAL struct ThreadLocalFibersContext
{
	Fiber threadFiber;
	Fiber *activeFiber;
} threadLocalFibersContext;

struct FibersContext
{
	s64 pageSize;
	s64 stackSize;
	s64 guardPageCount;
	char *fiberStackMemory;
	volatile s64 fiberCount;
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

void CreateFiber(Fiber *fiber, FiberProcedure procedure, void *parameter)
{
	getcontext(&fiber->context);
	auto fiberIndex = AtomicFetchAndAdd(&fibersContext.fiberCount, 1);
	auto stack = (char *)AllocateAlignedMemory(fibersContext.pageSize, fibersContext.stackSize + (2 * fibersContext.guardPageCount * fibersContext.pageSize));
	if (mprotect(stack, (fibersContext.guardPageCount * fibersContext.pageSize), PROT_NONE) == -1)
	{
		Abort("mprotect failed for fiber index %d: %k.", fiberIndex, GetPlatformError());
	}
	fiber->context.uc_stack.ss_sp = stack + (fibersContext.guardPageCount * fibersContext.pageSize);
	fiber->context.uc_stack.ss_size = fibersContext.stackSize;
	fiber->context.uc_link = 0;
	auto temporaryContext = ucontext_t{};
	auto fiberCreationInfo = FiberCreationInfo
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

void InitializeFibers()
{
	fibersContext.pageSize = GetPageSize();
	fibersContext.stackSize = 40 * fibersContext.pageSize;
	fibersContext.guardPageCount = 1;
	fibersContext.fiberCount = 0;
}
