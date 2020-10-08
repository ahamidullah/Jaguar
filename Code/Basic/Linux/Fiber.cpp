#include "../Fiber.h"
#include "../CPU.h"
#include "../Thread.h"
#include "../Log.h"
#include "../Atomic.h"
#include "../Memory.h"
#include "../Process.h"
#include "../Time.h"
#include "../Pool.h"

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
const auto FiberStackPlusGuardSize = FiberStackSize + (FiberStackGuardPageCount * CPUPageSize());

auto fiberStackPool = NewValuePoolIn<u8 *>(GlobalAllocator(), 128);

#if __x86_64__
#if 0
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

		__asm(R"(
		.type SetPlatformContext, @function
		.global SetPlatformContext
		SetPlatformContext:
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

		__asm(R"(
		.type SwapPlatformContext, @function
		.global SwapPlatformContext
		SwapPlatformContext:
			# Save the return address.
			movq (%rsp), %r8
			movq %r8, 8*0(%rdi) // RIP
			leaq 8(%rsp), %r8
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
#endif

#if 0
__attribute__((section(".text")))
static u8 SetPlatformContextCode[] =
{
0x4c, 0x8b, 0x07,
0x48, 0x8b, 0x67, 0x08,
0x48, 0x8b, 0x5f, 0x10,
0x48, 0x8b, 0x6f, 0x18,
0x4c, 0x8b, 0x67, 0x20,
0x4c, 0x8b, 0x6f, 0x28,
0x4c, 0x8b, 0x77, 0x30,
0x4c, 0x8b, 0x7f, 0x38,
0x41, 0x50,
0x31, 0xc0,
0xc3,
};

__attribute__((section(".text")))
static u8 SwapPlatformContextCode[] =
{
#if 0
	0x4c, 0x8b, 0x04, 0x24,
	0x4c, 0x89, 0x07,
	0x4c, 0x8d, 0x44, 0x24, 0x08,
	0x4c, 0x89, 0x47, 0x08,
	0x48, 0x89, 0x5f, 0x10,
	0x4c, 0x8b, 0x45, 0x00,
	0x4c, 0x89, 0x47, 0x18,
	0x4c, 0x89, 0x67, 0x20,
	0x4c, 0x89, 0x6f, 0x28,
	0x4c, 0x89, 0x77, 0x30,
	0x4c, 0x89, 0x7f, 0x38,
	0x4c, 0x8b, 0x06,
	0x48, 0x8b, 0x66, 0x08,
	0x48, 0x8d, 0x64, 0x24, 0xf8,
	0x48, 0x8b, 0x5e, 0x10,
	0x48, 0x8b, 0x6e, 0x18,
	0x4c, 0x8b, 0x66, 0x20,
	0x4c, 0x8b, 0x6e, 0x28,
	0x4c, 0x8b, 0x76, 0x30,
	0x4c, 0x8b, 0x7e, 0x38,
	0x48, 0x89, 0xd7,
	0x41, 0x50,
	0x31, 0xc0,
	0xc3,
#endif
0x4c, 0x8b, 0x04, 0x24,
0x4c, 0x89, 0x07,
0x4c, 0x8d, 0x44, 0x24, 0x08,
0x4c, 0x89, 0x47, 0x08,
0x48, 0x89, 0x5f, 0x10,
0x48, 0x89, 0x6f, 0x18,
0x4c, 0x89, 0x67, 0x20,
0x4c, 0x89, 0x6f, 0x28,
0x4c, 0x89, 0x77, 0x30,
0x4c, 0x89, 0x7f, 0x38,
0x4c, 0x8b, 0x06,
0x48, 0x8b, 0x66, 0x08,
0x48, 0x8b, 0x5e, 0x10,
0x48, 0x8b, 0x6e, 0x18,
0x4c, 0x8b, 0x66, 0x20,
0x4c, 0x8b, 0x6e, 0x28,
0x4c, 0x8b, 0x76, 0x30,
0x4c, 0x8b, 0x7e, 0x38,
0x48, 0x89, 0xd7,
0x41, 0x50,
0x31, 0xc0,
0xc3,
};
#endif

__attribute__((section(".text")))
static u8 GetSystemContextCode[] =
{
	0x4c, 0x8b, 0x04, 0x24,
	0x4c, 0x89, 0x07,
	0x4c, 0x8d, 0x44, 0x24, 0x08,
	0x4c, 0x89, 0x47, 0x08,
	0x48, 0x89, 0x5f, 0x10,
	0x48, 0x89, 0x6f, 0x18,
	0x4c, 0x89, 0x67, 0x20,
	0x4c, 0x89, 0x6f, 0x28,
	0x4c, 0x89, 0x77, 0x30,
	0x4c, 0x89, 0x7f, 0x38,
	0x31, 0xc0,
	0xc3,
};

__attribute__((section(".text")))
static u8 SetSystemContextCode[] =
{
	0x4c, 0x8b, 0x07,
	0x48, 0x8b, 0x67, 0x08,
	0x48, 0x8b, 0x5f, 0x10,
	0x48, 0x8b, 0x6f, 0x18,
	0x4c, 0x8b, 0x67, 0x20,
	0x4c, 0x8b, 0x6f, 0x28,
	0x4c, 0x8b, 0x77, 0x30,
	0x4c, 0x8b, 0x7f, 0x38,
	0x41, 0x50,
	0x31, 0xc0,
	0xc3,
};

__attribute__((section(".text")))
static u8 SwapSystemContextCode[] =
{
	0x4c, 0x8b, 0x04, 0x24,
	0x4c, 0x89, 0x07,
	0x4c, 0x8d, 0x44, 0x24, 0x08,
	0x4c, 0x89, 0x47, 0x08,
	0x48, 0x89, 0x5f, 0x10,
	0x48, 0x89, 0x6f, 0x18,
	0x4c, 0x89, 0x67, 0x20,
	0x4c, 0x89, 0x6f, 0x28,
	0x4c, 0x89, 0x77, 0x30,
	0x4c, 0x89, 0x7f, 0x38,
	0x4c, 0x8b, 0x06,
	0x48, 0x8b, 0x66, 0x08,
	0x48, 0x8b, 0x5e, 0x10,
	0x48, 0x8b, 0x6e, 0x18,
	0x4c, 0x8b, 0x66, 0x20,
	0x4c, 0x8b, 0x6e, 0x28,
	0x4c, 0x8b, 0x76, 0x30,
	0x4c, 0x8b, 0x7e, 0x38,
	0x41, 0x50,
	0x31, 0xc0,
	0xc3,
};

__attribute__((section(".text")))
static u8 StartSystemContextCode[] =
{
	0x4c, 0x8b, 0x04, 0x24,
	0x4c, 0x89, 0x07,
	0x4c, 0x8d, 0x44, 0x24, 0x08,
	0x4c, 0x89, 0x47, 0x08,
	0x48, 0x89, 0x5f, 0x10,
	0x48, 0x89, 0x6f, 0x18,
	0x4c, 0x89, 0x67, 0x20,
	0x4c, 0x89, 0x6f, 0x28,
	0x4c, 0x89, 0x77, 0x30,
	0x4c, 0x89, 0x7f, 0x38,
	0x4c, 0x8b, 0x06,
	0x48, 0x8b, 0x66, 0x08,
	0x48, 0x89, 0xd7,
	0x41, 0x50,
	0x31, 0xc0,
	0xc3,
};


	static void (*GetSystemContext)(SystemContext *) = (void (*)(SystemContext *))GetSystemContextCode;
	static void (*SetSystemContext)(SystemContext *) = (void (*)(SystemContext *))SetSystemContextCode;
	static void (*SwapSystemContext)(SystemContext *, SystemContext *) = (void (*)(SystemContext *, SystemContext *))SwapSystemContextCode;
	static void (*StartSystemContext)(SystemContext *, SystemContext *, void *) = (void (*)(SystemContext *, SystemContext *, void *))StartSystemContextCode;

#if 0
	// @TODO: NOT FIBER PROC
	void RunPlatformContext(SystemContext *from, SystemContext *to, FiberProcedure proc, void *param, void *stack, s64 stackSize)
	{
		auto s = (u8 *)stack;
		// The stack grows down!
		s += stackSize;
  		// Make 128 byte scratch space for the Red Zone. This arithmetic will not unalign
  		// our stack pointer because 128 is a multiple of 16. The Red Zone must also be
  		// 16-byte aligned.
  		s -= 128;
		// Make some room for the parameter.
		auto p = (DoRunPlatformContextParameter *)s;
		s -= sizeof(DoRunPlatformContextParameter);
		// Align stack pointer on 16-byte boundary, required for SysV and SSE.
  		s = (u8*)((uintptr_t)s & -16L);
  		// For some reason, SSE segfaults unless I move the stack pointer down by 8 bytes?
  		// @TODO: WHY????????
  		s -= 8;
		//to->rip = proc;
		to->rsp = s;
		DoRunSystemContext(from, to, param);
	}
	
	void SwapSystemContext(SystemContext *from, SystemContext *to)
	{
		auto s = (u8 *)stack;
		// The stack grows down!
		s += stackSize;
  		// Make 128 byte scratch space for the Red Zone. This arithmetic will not unalign
  		// our stack pointer because 128 is a multiple of 16. The Red Zone must also be
  		// 16-byte aligned.
  		s -= 128;
		// Make some room for the parameter.
		auto p = (DoRunPlatformContextParameter *)s;
		s -= sizeof(DoRunPlatformContextParameter);
		// Align stack pointer on 16-byte boundary, required for SysV and SSE.
  		s = (u8*)((uintptr_t)s & -16L);
  		// For some reason, SSE segfaults unless I move the stack pointer down by 8 bytes?
  		// @TODO: WHY????????
  		s -= 8;
  		if (to->started)
  		{
  			DoSwapSystemContext(from, to);
  		}
  		else
  		{
  			to->started = true;
  			from->started = true;
			DoRunSystemContext(from, to, to->param);
		}
	}
#endif
	typedef void (*SystemContextProcedure)(void *);

	struct RunSystemContextParameter
	{
		SystemContextProcedure procedure;
		void *parameter;
		SystemContext *callingContext;
		SystemContext *thisContext;
	};

	/*
	void RunSystemContext(void *param)
	{
		auto p = (RunSystemContextParameter *)param;
		// Save the parameters to the stack before swapping back to the calling context.
		// Once we swap back, the param pointer will be invalid because we stored the parameters on the stack.
		auto pr = p->procedure;
		auto pm = p->parameter;
		SwapSystemContext(p->thisContext, p->callingContext);
		pr(pm);
		pthread_exit(NULL);
	}
	*/

	void RunSystemContext(SystemContext *from, SystemContext *to, SystemContextProcedure proc, void *param)
	{
		to->rip = (void *)proc;
  		StartSystemContext(from, to, param);
	}

	SystemContext NewSystemContext(void *stack, s64 stackSize)
	{
		auto s = (u8 *)stack;
		// The stack grows down!
		s += stackSize;
		// Align stack pointer on 16-byte boundary, required for SysV and SSE.
  		s = (u8 *)((uintptr_t)s & -16L);
  		// Make 128 byte scratch space for the Red Zone. This arithmetic will not unalign
  		// our stack pointer because 128 is a multiple of 16. The Red Zone must also be
  		// 16-byte aligned.
  		s -= 128;
  		// For some reason, SSE segfaults unless I move the stack pointer down by 8 bytes.
  		// @TODO: Why????????
  		s -= 8;
  		return
  		{
  			.rsp = s,
  		};
	}
	/*
	SystemContext NewSystemContext(SystemContextProcedure proc, void *param, void *stack, s64 stackSize)
	{
		auto s = (u8 *)stack;
		// The stack grows down!
		s += stackSize;
		// Align stack pointer on 16-byte boundary, required for SysV and SSE.
  		s = (u8 *)((uintptr_t)s & -16L);
  		// Make 128 byte scratch space for the Red Zone. This arithmetic will not unalign
  		// our stack pointer because 128 is a multiple of 16. The Red Zone must also be
  		// 16-byte aligned.
  		s -= 128;
  		// For some reason, SSE segfaults unless I move the stack pointer down by 8 bytes.
  		// @TODO: Why????????
  		s -= 8;
  		auto to = SystemContext
  		{
  			.rip = (void *)RunSystemContext,
  			.rsp = s,
  		};
  		auto from = SystemContext{};
  		auto p = RunSystemContextParameter
  		{
  			.procedure = proc,
  			.parameter = param,
  			.callingContext = &from,
  			.thisContext = &to,
  		};
  		StartSystemContext(&from, &to, &p);
  		return to;
	}
	*/

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

struct RunFiberParameter
{
	FiberProcedure procedure;
	void *parameter;
	SystemContext *callingContext;
	SystemContext *thisContext;
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

void RunFiber(void *param)
{
	auto p = (RunFiberParameter *)param;
	// Save the parameters to the stack before swapping back to the calling context.
	// Once we swap back, the param pointer will be invalid because we stored the parameters on the stack.
	auto pr = p->procedure;
	auto pm = p->parameter;
	SwapSystemContext(p->thisContext, p->callingContext);
	pr(pm);
	pthread_exit(NULL);
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
	auto f = Fiber
	{
		.contextAllocator = GlobalAllocator(),
		.contextAllocatorStack = NewStackIn<Allocator *>(GlobalAllocator(), 0),
	};
	auto stack = (u8 *)AllocatePlatformMemory(FiberStackPlusGuardSize);
	Assert(AlignPointer(stack, CPUPageSize()) == stack);
	if (mprotect(stack, (FiberStackGuardPageCount * CPUPageSize()), PROT_NONE) == -1)
	{
		Abort("Fiber", "Failed to mprotect stack guard page: %k.", PlatformError());
	}
	f.context = NewSystemContext(stack, FiberStackPlusGuardSize);
	auto cc = SystemContext{};
	auto p = RunFiberParameter
	{
		.procedure = proc,
		.parameter = param,
		.callingContext = &cc,
		.thisContext = &f.context,
	};
	RunSystemContext(&cc, &f.context, RunFiber, &p);
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
	auto from = RunningFiber();
	SetRunningFiber(this);
	SwapSystemContext(&from->context, &this->context);
	#if 0
	if (this->runParameters.running)
	{
		// This fiber is already running, just resume it's execution.
	}
	else
	{
		this->runParameters.running = true;
		//this->stack = fiberStackPool.Get();
		//if (!this->stack)
		if (true)
		{
			this->stack = (u8 *)AllocatePlatformMemory(FiberStackPlusGuardSize);
			Assert(AlignPointer(this->stack, CPUPageSize()) == this->stack);
			if (mprotect(this->stack, (FiberStackGuardPageCount * CPUPageSize()), PROT_NONE) == -1)
			{
				Abort("Fiber", "Failed to mprotect stack guard page: %k.", PlatformError());
			}
		}
		RunPlatformContext(&from->context, &this->context, RunFiber, &this->runParameters, this->stack, FiberStackPlusGuardSize);
	}
	SetRunningFiber(from);
	if (!this->runParameters.running)
	{
		//fiberStackPool.Release(this->stack);
	}
	#endif
	/*
	auto from = RunningFiber();
	SetRunningFiber(this);
	if (this->runParameters.running)
	{
		// This fiber is already running, just resume it's execution.
		SwapSystemContext(&from->context, &this->context);
	}
	else
	{
		this->runParameters.running = true;
		//this->stack = fiberStackPool.Get();
		//if (!this->stack)
		if (true)
		{
			this->stack = (u8 *)AllocatePlatformMemory(FiberStackPlusGuardSize);
			Assert(AlignPointer(this->stack, CPUPageSize()) == this->stack);
			if (mprotect(this->stack, (FiberStackGuardPageCount * CPUPageSize()), PROT_NONE) == -1)
			{
				Abort("Fiber", "Failed to mprotect stack guard page: %k.", PlatformError());
			}
		}
		RunPlatformContext(&from->context, &this->context, RunFiber, &this->runParameters, this->stack, FiberStackPlusGuardSize);
	}
	SetRunningFiber(from);
	if (!this->runParameters.running)
	{
		//fiberStackPool.Release(this->stack);
	}
	*/
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
