#include "../Fiber.h"
#include "../Thread.h"
#include "../Atomic.h"
#include "../Memory.h"

#if defined(THREAD_SANITIZER_BUILD)
	#include <sanitizer/tsan_interface.h>
#endif

// @TODO:
//    __tsan_get_current_fiber()
//    __tsan_destroy_fiber()
//    __tsan_set_fiber_name()

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

struct FiberGlobals
{
	s64 pageSize;
	s64 stackSize;
	s64 guardPageCount;
	char *fiberStackMemory;
	volatile s64 fiberCount;
} fiberGlobals;

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
		auto context = ucontext_t{};
		swapcontext(&context, fiberCreationInfo->callingContext);
	}
	procedure(parameter);
	pthread_exit(NULL);
}

void CreateFiber(Fiber *fiber, FiberProcedure procedure, void *parameter)
{
	getcontext(&fiber->context);
	auto fiberIndex = AtomicFetchAndAdd(&fiberGlobals.fiberCount, 1);
	auto stack = (char *)AllocateAlignedMemory(fiberGlobals.pageSize, fiberGlobals.stackSize + (2 * fiberGlobals.guardPageCount * fiberGlobals.pageSize)); // @TODO
	if (mprotect(stack, (fiberGlobals.guardPageCount * fiberGlobals.pageSize), PROT_NONE) == -1)
	{
		Abort("mprotect failed for fiber index %d: %k.", fiberIndex, GetPlatformError());
	}
	fiber->context.uc_stack.ss_sp = stack + (fiberGlobals.guardPageCount * fiberGlobals.pageSize);
	fiber->context.uc_stack.ss_size = fiberGlobals.stackSize;
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

#if defined(THREAD_SANITIZER_BUILD)
	fiber->tsanFiber = __tsan_create_fiber(0);
#endif
}

void ConvertThreadToFiber(Fiber *fiber)
{
	threadLocalFibersContext.activeFiber = fiber;

#if defined(THREAD_SANITIZER_BUILD)
		fiber->tsanFiber = __tsan_create_fiber(0);
#endif
}

// @TODO: Prevent two fibers from running at the same time.
void SwitchToFiber(Fiber *fiber)
{
	if (!_setjmp(threadLocalFibersContext.activeFiber->jumpBuffer))
	{
#if defined(THREAD_SANITIZER_BUILD)
		Assert(fiber->tsanFiber);
		__tsan_switch_to_fiber(fiber->tsanFiber, 0);
#endif

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
	fiberGlobals.pageSize = GetPageSize();
	fiberGlobals.stackSize = 100 * fiberGlobals.pageSize;
	fiberGlobals.guardPageCount = 1;
	fiberGlobals.fiberCount = 0;
	LogPrint(
		INFO_LOG,
		"Fiber info:\n"
		"	pageSize: %d\n"
		"	stackSize: %d\n"
		"	guardPageCount: %d\n",
		fiberGlobals.pageSize, fiberGlobals.stackSize, fiberGlobals.guardPageCount);
}
