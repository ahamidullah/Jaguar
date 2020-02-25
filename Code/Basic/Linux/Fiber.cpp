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

static char *fiberStackMemory;
static volatile s32 fiberCount;
static size_t pageSize;

static THREAD_LOCAL Fiber threadFiber;
static THREAD_LOCAL Fiber *activeFiber;

struct FiberCreationInfo
{
	FiberProcedure procedure;
	void *parameter;
	jmp_buf *jumpBuffer;
	ucontext_t *callingContext;
};

void RunFiber(void *fiberCreationInfoPointer)
{
	FiberCreationInfo *fiberCreationInfo = (FiberCreationInfo *)fiberCreationInfoPointer;
	FiberProcedure procedure = fiberCreationInfo->procedure;
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
void CreateFiber(Fiber *fiber, FiberProcedure procedure, void *parameter)
{
	getcontext(&fiber->context);
	s32 fiberIndex = AtomicFetchAndAdd(&fiberCount, 1);
	fiber->context.uc_stack.ss_sp = fiberStackMemory + ((fiberIndex * FIBER_STACK_SIZE) + ((fiberIndex + 1) * pageSize));
	fiber->context.uc_stack.ss_size = FIBER_STACK_SIZE;
	fiber->context.uc_link = 0;
	ucontext_t temporaryContext;
	FiberCreationInfo fiberCreationInfo = {
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
	activeFiber = fiber;
}

// @TODO: Prevent two fibers from running at the same time.
void SwitchToFiber(Fiber *fiber)
{
	if (!_setjmp(activeFiber->jumpBuffer))
	{
		activeFiber = fiber;
		_longjmp(fiber->jumpBuffer, 1);
	}
}

Fiber *GetCurrentFiber()
{
	return activeFiber;
}

void InitializeFiber(u32 maxFiberCount)
{
	pageSize = GetPageSize();
	fiberStackMemory = (char *)AllocateMemory((maxFiberCount * FIBER_STACK_SIZE) + ((maxFiberCount + 1) * pageSize));
	for (s32 i = 0; i <= maxFiberCount; i++)
	{
		mprotect(fiberStackMemory + ((i * FIBER_STACK_SIZE) + (i * pageSize)), pageSize, PROT_NONE);
	}
}
