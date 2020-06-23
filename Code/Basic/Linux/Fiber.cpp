#include "../Fiber.h"
#include "../CPU.h"
#include "../Thread.h"
#include "../Log.h"
#include "../Atomic.h"
#include "../Memory.h"
#include "../Process.h"

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

auto fiberStackSize = 100 * CPUPageSize();
auto fiberStackGuardPageCount = 1;
volatile auto numFibers = 0;
THREAD_LOCAL auto runningFiber = (Fiber *){};

void InitializeFibers()
{
	LogPrint(
		LogLevelInfo,
		"Fiber",
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

void RunFiber(void *p)
{
	auto fci = (FiberCreationInfo *)p;
	auto proc = fci->procedure;
	auto param = fci->parameter;
	if (!_setjmp(*fci->jumpBuffer))
	{
		auto context = ucontext_t{};
		swapcontext(&context, fci->callingContext);
	}
	proc(param);
	pthread_exit(NULL);
}

void CreateFiber(Fiber *f, FiberProcedure proc, void *param)
{
	getcontext(&f->context);
	auto index = AtomicFetchAndAdd(&numFibers, 1);
	auto stack = (char *)AllocateAlignedMemory(CPUPageSize(), fiberStackSize + (2 * fiberStackGuardPageCount * CPUPageSize()));
	if (mprotect(stack, (fiberStackGuardPageCount * CPUPageSize()), PROT_NONE) == -1)
	{
		Abort("mprotect failed for fiber index %d: %k.", index, GetPlatformError());
	}
	f->context.uc_stack.ss_sp = stack + (fiberStackGuardPageCount * CPUPageSize());
	f->context.uc_stack.ss_size = fiberStackSize;
	f->context.uc_link = 0;
	auto tempContext = ucontext_t{};
	auto fci = FiberCreationInfo
	{
		.procedure = proc,
		.parameter = param,
		.jumpBuffer = &f->jumpBuffer,
		.callingContext = &tempContext,
	};
	makecontext(&f->context, (void(*)())RunFiber, 1, &fci);
	swapcontext(&tempContext, &f->context);
	#if defined(THREAD_SANITIZER_BUILD)
		f->tsanFiber = __tsan_create_fiber(0);
	#endif
}

void ConvertThreadToFiber(Fiber *f)
{
	runningFiber = f;
	#if defined(THREAD_SANITIZER_BUILD)
		f->tsanFiber = __tsan_create_fiber(0);
	#endif
}

// @TODO: Prevent two fibers from running at the same time.
void SwitchToFiber(Fiber *f)
{
	if (!_setjmp(runningFiber->jumpBuffer))
	{
		#if defined(THREAD_SANITIZER_BUILD)
			__tsan_switch_to_fiber(f->tsanFiber, 0);
		#endif
		runningFiber = f;
		_longjmp(f->jumpBuffer, 1);
	}
}
