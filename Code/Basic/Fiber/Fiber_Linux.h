#pragma once

#include "basic/container/array.h"
#include "Basic/PCH.h"
#include "Basic/Memory.h"
#include "Common.h"

namespace fiber
{

	struct SystemContext
	{
		void *rip, *rsp;
		void *rbx, *rbp, *r12, *r13, *r14, *r15;
	};

#define NEW_FIBER 1

#if !NEW_FIBER
#include <setjmp.h>
#endif

typedef void (*Procedure)(void *);

struct Fiber
{
#if !NEW_FIBER
	ucontext_t context;
	jmp_buf jumpBuffer;
	mem::Allocator *contextAllocator;
	arr::array<mem::allocator *> contextAllocatorStack;
#else
	SystemContext context;
	mem::Allocator *contextAllocator;
	arr::array<mem::allocator *> contextAllocatorStack;
#endif
	#ifdef ThreadSanitizerBuild
		void *tsan;
	#endif

	void Switch();
	void Delete();
};

Fiber New(Procedure proc, void *param);
void ConvertThread(Fiber *f);
Fiber *Current();

}
