#include "../Basic.h"

// ucontext_t is the recommended method for implementing fibers on Linux, but it is apparently very
// slow because it preserves each fiber's signal mask.
//
// Our method (from http://www.1024cores.net/home/lock-free-algorithms/tricks/fibers) still creates
// a ucontext_t, but it uses _setjmp/_longjmp to switch between them, thus avoiding syscalls for
// saving and setting signal masks.  It works by swapping to the fiber's context during fiber
// creation, setting up a long jump into the new fiber's context using _setjmp, then swapping back
// to the calling context.  Now, when we want to switch to the fiber, we can _longjmp directly into
// the fiber's context. 

// @TODO:
//    __tsan_get_current_fiber()
//    __tsan_destroy_fiber()
//    __tsan_set_fiber_name()

auto fiberStackSize = 100 * GetCPUPageSize();
auto fiberStackGuardPageCount = 1;
volatile auto fiberCount = 0;
THREAD_LOCAL auto runningFiber = (Fiber *){};

void InitializeFibers()
{
	LogPrint(
		INFO_LOG,
		"Fiber info:\n"
		"	stackSize: %d\n"
		"	guardPageCount: %d\n",
		fiberStackSize, fiberStackGuardPageCount);
}

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
	auto fiberIndex = AtomicFetchAndAdd(&fiberCount, 1);
	auto fiberStack = (char *)AllocateAlignedMemory(GetCPUPageSize(), fiberStackSize + (2 * fiberStackGuardPageCount * GetCPUPageSize()));
	if (mprotect(fiberStack, (fiberStackGuardPageCount * GetCPUPageSize()), PROT_NONE) == -1)
	{
		Abort("mprotect failed for fiber index %d: %k.", fiberIndex, GetPlatformError());
	}
	fiber->context.uc_stack.ss_sp = fiberStack + (fiberStackGuardPageCount * GetCPUPageSize());
	fiber->context.uc_stack.ss_size = fiberStackSize;
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
	runningFiber = fiber;
	#if defined(THREAD_SANITIZER_BUILD)
		fiber->tsanFiber = __tsan_create_fiber(0);
	#endif
}

// @TODO: Prevent two fibers from running at the same time.
void SwitchToFiber(Fiber *fiber)
{
	if (!_setjmp(runningFiber->jumpBuffer))
	{
		#if defined(THREAD_SANITIZER_BUILD)
			__tsan_switch_to_fiber(fiber->tsanFiber, 0);
		#endif
		runningFiber = fiber;
		_longjmp(fiber->jumpBuffer, 1);
	}
}
