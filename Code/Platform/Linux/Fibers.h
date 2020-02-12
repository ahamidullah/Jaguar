#pragma once

#include <ucontext.h>
#include <setjmp.h>

struct PlatformFiber {
	ucontext_t context;
	jmp_buf jumpBuffer;
};

typedef void (*PlatformFiberProcedure)(void *);
