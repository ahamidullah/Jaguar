#include "ContextAllocator.h"
#include "Basic/Container/Array.h"
#include "../Fiber.h"

namespace mem
{

// We want these variables to have constant initialization so other global variable initializers can use the context allocator.
ThreadLocal auto contextAllocator = (Allocator *){};
ThreadLocal auto contextAllocatorStack = arr::array<Allocator *>{};

Allocator *ContextAllocator()
{
	if (RunningFiber())
	{
		return RunningFiber()->contextAllocator;
	}
	if (!contextAllocator)
	{
		return GlobalHeap();
	}
	return contextAllocator;
}

void PushContextAllocator(Allocator *a)
{
	if (RunningFiber())
	{
		RunningFiber()->contextAllocatorStack.Append(a);
		RunningFiber()->contextAllocator = a;
		return;
	}
	contextAllocatorStack.Append(a);
	contextAllocator = a;
}

void PopContextAllocator()
{
	auto stk = (arr::array<Allocator *> *){};
	auto ctx = (Allocator **){};
	if (RunningFiber())
	{
		stk = &RunningFiber()->contextAllocatorStack;
		ctx = &RunningFiber()->contextAllocator;
	}
	else
	{
		stk = &contextAllocatorStack;
		ctx = &contextAllocator;
	}
	if (stk->count == 0)
	{
		log::Error("Memory", "Tried to pop an empty context allocator stack.\n");
		return;
	}
	stk->Pop();
	if (stk->count == 0)
	{
		*ctx = GlobalHeap();
	}
	else
	{
		*ctx = *stk->Last();
	}
}

}
