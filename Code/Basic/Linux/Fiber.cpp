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
//    __tsan_set_fiber_name()

const auto FiberStackSize = 100 * CPUPageSize();
const auto FiberStackGuardPageCount = 1;

Fiber **RunningFiberPointer()
{
	static ThreadLocal auto rf = (Fiber *){};
	return &rf;
}

Fiber *RunningFiber()
{
	return *RunningFiberPointer();
}

void SetRunningFiber(Fiber *f)
{
	*RunningFiberPointer() = f;
}

//void InitializeFibers()
//{
//	LogInfo("Fiber", "Stack size: %d", FiberStackSize);
//	LogInfo("Fiber", "Guard pages: %d", FiberStackGuardPageCount);
//}

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

Fiber *NewFiber(FiberProcedure proc, void *param)
{
	auto f = (Fiber *)GlobalHeap()->Allocate(sizeof(Fiber));
	f->contextAllocatorStack.SetAllocator(GlobalHeap());
	f->contextAllocator = GlobalHeap();
	getcontext(&f->context);
	auto stack = (char *)GlobalHeap()->AllocateAligned(FiberStackSize + (FiberStackGuardPageCount * CPUPageSize()), CPUPageSize());
	if (mprotect(stack, (FiberStackGuardPageCount * CPUPageSize()), PROT_NONE) == -1)
	{
		Abort("Fiber", "Failed to mprotect stack guard page: %k.", PlatformError());
	}
	f->context.uc_stack.ss_sp = stack + (FiberStackGuardPageCount * CPUPageSize());
	f->context.uc_stack.ss_size = FiberStackSize;
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
	#ifdef ThreadSanitizerBuild
		f->tsan = __tsan_create_fiber(0);
	#endif
	return f;
}

Fiber *ConvertThreadToFiber()
{
	auto f = (Fiber *)GlobalHeap()->Allocate(sizeof(Fiber));
	SetRunningFiber(f);
	#ifdef ThreadSanitizerBuild
		f->tsan = __tsan_create_fiber(0);
	#endif
	return f;
}

// @TODO: Prevent two fibers from running at the same time.
void Fiber::Switch()
{
	if (!_setjmp(RunningFiber()->jumpBuffer))
	{
		#ifdef ThreadSanitizerBuild
			__tsan_switch_to_fiber(this->tsan, 0);
		#endif
		SetRunningFiber(this);
		_longjmp(this->jumpBuffer, 1);
	}
}

void Fiber::Delete()
{
	if (this == RunningFiber())
	{
		Abort("Fiber", "Attempted to delete running fiber.");
	}
	#ifdef ThreadSanitizerBuild
		__tsan_destroy_fiber(this->tsan);
	#endif
	auto stack = (char *)this->context.uc_stack.ss_sp - (FiberStackGuardPageCount * CPUPageSize());
	if (mprotect(stack, (FiberStackGuardPageCount * CPUPageSize()), PROT_READ | PROT_WRITE) == -1)
	{
		Abort("Fiber", "Failed to undo mprotect while deleting fiber: %k.", PlatformError());
	}
	GlobalHeap()->Deallocate(stack);
	GlobalHeap()->Deallocate(this);
}
