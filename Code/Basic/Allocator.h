#pragma once

#include "Common.h"

struct Allocator
{
	virtual void *Allocate(s64 size) = 0;
	virtual void *AllocateAligned(s64 size, s64 align) = 0;
	virtual void *Resize(void *mem, s64 size) = 0;
	virtual void Deallocate(void *mem) = 0;
	virtual void Clear() = 0;
	virtual void Free() = 0;
};
