#include "../Fiber.h"
#include "../CPU.h"
#include "../Thread.h"
#include "../Log.h"
#include "../Atomic.h"
#include "../Memory.h"
#include "../Process.h"
#include "../Time.h"

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

#if __x86_64__
	__asm(R"(
		.type GetPlatformContext, @function
		.global GetPlatformContext
		GetPlatformContext:
			# Save the return address and stack pointer.
			movq (%rsp), %r8
			movq %r8, 8*0(%rdi) // RIP
			leaq 8(%rsp), %r8
			movq %r8, 8*1(%rdi) // RSP
			# Save preserved registers.
			movq %rbx, 8*2(%rdi)
			movq %rbp, 8*3(%rdi)
			movq %r12, 8*4(%rdi)
			movq %r13, 8*5(%rdi)
			movq %r14, 8*6(%rdi)
			movq %r15, 8*7(%rdi)
			# Return.
			xorl %eax, %eax
			ret)");

		void SetPlatformContext(PlatformContext *c)
		{
			__asm(R"(
			# Should return to the address set with {get, swap}_context.
			movq 8*0(%rdi), %r8
			# Load new stack pointer.
			movq 8*1(%rdi), %rsp
			# Load preserved registers.
			movq 8*2(%rdi), %rbx
			movq 8*3(%rdi), %rbp
			movq 8*4(%rdi), %r12
			movq 8*5(%rdi), %r13
			movq 8*6(%rdi), %r14
			movq 8*7(%rdi), %r15
			# Push RIP to stack for RET.
			pushq %r8
			# Return.
			xorl %eax, %eax
			ret)");
		}

		void SwapPlatformContext(PlatformContext *from, PlatformContext *to, void *arg)
		{
			__asm(R"(
			# Save the return address.
			movq 8(%rsp), %r8
			movq %r8, 8*0(%rdi) // RIP
			leaq 16(%rsp), %r8
			movq %r8, 8*1(%rdi) // RSP
			# Save preserved registers.
			movq %rbx, 8*2(%rdi)
			movq (%rbp), %r8
			movq %r8, 8*3(%rdi)
			movq %r12, 8*4(%rdi)
			movq %r13, 8*5(%rdi)
			movq %r14, 8*6(%rdi)
			movq %r15, 8*7(%rdi)
			# Should return to the address set with {get, swap}_context.
			movq 8*0(%rsi), %r8
			# Load new stack pointer.
			movq 8*1(%rsi), %rsp
			# Align the pointer for some reason, not sure.
			leaq -0x8(%rsp), %rsp
			# Load preserved registers.
			movq 8*2(%rsi), %rbx
			movq 8*3(%rsi), %rbp
			movq 8*4(%rsi), %r12
			movq 8*5(%rsi), %r13
			movq 8*6(%rsi), %r14
			movq 8*7(%rsi), %r15
			# Set the argument.
			movq %rdx, %rdi
			# Push RIP to stack for RET.
			pushq %r8
			# Return.
			xorl %eax, %eax
			ret)");
		}

	struct DoRunPlatformContextParameter
	{
		FiberProcedure procedure;
		void *parameter;
		PlatformContext *callingContext;
	};

	void DoRunPlatformContext(void *param)
	{
		auto p = (DoRunPlatformContextParameter *)param;
		p->procedure(p->parameter);
		SetPlatformContext(p->callingContext);
	}

	// @TODO: NOT FIBER PROC
	void RunPlatformContext(PlatformContext *from, PlatformContext *to, FiberProcedure proc, void *param, void *stack)
	{
		auto p = DoRunPlatformContextParameter
		{
			.procedure = proc,
			.parameter = param,
			.callingContext = from,
		};
		to->rip = (void *)DoRunPlatformContext;
		to->rsp = stack;
		SwapPlatformContext(from, to, &p);
	}
#else
	#error Fiber: context switching is not defined for this CPU architecture.
#endif

Fiber **RunningFiberPointer()
{
	static ThreadLocal auto f = (Fiber *){};
	return &f;
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

#if !NEW_FIBER
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
#else

// @TODO: Move this somewhere else. x64 and maybe linux specific?

void RunFiber(void *params)
{
	auto p = (RunFiberParameters *)params;
	p->procedure(p->parameter);
}

#endif

Fiber NewFiber(FiberProcedure proc, void *param)
{
#if !NEW_FIBER
	auto f = Fiber{};
	f.contextAllocatorStack.SetAllocator(GlobalAllocator());
	f.contextAllocator = GlobalAllocator();
	getcontext(&f.context);
	auto stack = (u8 *)AllocatePlatformMemory(FiberStackSize + (FiberStackGuardPageCount * CPUPageSize()));
	Assert(AlignPointer(stack, CPUPageSize()) == stack);
	if (mprotect(stack, (FiberStackGuardPageCount * CPUPageSize()), PROT_NONE) == -1)
	{
		Abort("Fiber", "Failed to mprotect stack guard page: %k.", PlatformError());
	}
	f.context.uc_stack.ss_sp = stack + (FiberStackGuardPageCount * CPUPageSize());
	f.context.uc_stack.ss_size = FiberStackSize;
	f.context.uc_link = 0;
	auto tempContext = ucontext_t{};
	auto fci = FiberCreationInfo
	{
		.procedure = proc,
		.parameter = param,
		.jumpBuffer = &f.jumpBuffer,
		.callingContext = &tempContext,
	};
	makecontext(&f.context, (void(*)())RunFiber, 1, &fci);
	auto t = NewTimer("ContextSwaps");
	swapcontext(&tempContext, &f.context);
	t.Print();
	#ifdef ThreadSanitizerBuild
		f.tsan = __tsan_create_fiber(0);
	#endif
	return f;
#else
	auto f = Fiber{};
	f.contextAllocatorStack.SetAllocator(GlobalAllocator());
	f.contextAllocator = GlobalAllocator();
	f.runParameters.procedure = proc;
	f.runParameters.parameter = param;
	#ifdef ThreadSanitizerBuild
		f.tsan = __tsan_create_fiber(0);
	#endif
	return f;
#endif
}

void ConvertThreadToFiber(Fiber *f)
{
	f->contextAllocatorStack.SetAllocator(GlobalAllocator());
	f->contextAllocator = GlobalAllocator();
	SetRunningFiber(f);
	#ifdef ThreadSanitizerBuild
		f->tsan = __tsan_create_fiber(0);
	#endif
}

// @TODO: Prevent two fibers from running at the same time.
void Fiber::Switch()
{
#if !NEW_FIBER
	if (!_setjmp(RunningFiber()->jumpBuffer))
	{
		#ifdef ThreadSanitizerBuild
			__tsan_switch_to_fiber(this->tsan, 0);
		#endif
		SetRunningFiber(this);
		_longjmp(this->jumpBuffer, 1);
	}
#else
	if (this->context.rsp) // @TODO
	{
		// This fiber is already running, just resume it's execution.
		SwapPlatformContext(&RunningFiber()->context, &this->context, 0);
	}
	else
	{
		auto totalStackSize = FiberStackSize + (FiberStackGuardPageCount * CPUPageSize());
		auto stack = (u8 *)AllocatePlatformMemory(totalStackSize);
		Assert(AlignPointer(stack, CPUPageSize()) == stack);
		if (mprotect(stack, (FiberStackGuardPageCount * CPUPageSize()), PROT_NONE) == -1)
		{
			Abort("Fiber", "Failed to mprotect stack guard page: %k.", PlatformError());
		}
		// The stack grows down!
		stack += totalStackSize;
		// Align stack pointer on 16-byte boundary, required for SysV and SSE.
  		stack = (u8*)((uintptr_t)stack & -16L);
  		// Make 128 byte scratch space for the Red Zone. This arithmetic will not unalign
  		// our stack pointer because 128 is a multiple of 16. The Red Zone must also be
  		// 16-byte aligned.
  		stack -= 128;
		RunPlatformContext(&RunningFiber()->context, &this->context, RunFiber, &this->runParameters, stack);
	}
#endif
}

/*
void Fiber::Delete()
{
	if (this == RunningFiber())
	{
		LogError("Fiber", "Attempted to delete running fiber.");
		return;
	}
	#ifdef ThreadSanitizerBuild
		__tsan_destroy_fiber(this->tsan);
	#endif
	auto stack = (char *)this->context.uc_stack.ss_sp - (FiberStackGuardPageCount * CPUPageSize());
	if (mprotect(stack, (FiberStackGuardPageCount * CPUPageSize()), PROT_READ | PROT_WRITE) == -1)
	{
		LogError("Fiber", "Failed to undo mprotect while deleting fiber: %k.", PlatformError());
	}
	GlobalAllocator()->Deallocate(stack);
	GlobalAllocator()->Deallocate(this);
}
*/
