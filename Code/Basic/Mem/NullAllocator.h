#pragma once

#include "Allocator.h"

namespace Memory
{

// @TODO: Get rid of the NullAllocator...

struct NullAllocator : Allocator
{
	void *Allocate(s64 size);
	void *AllocateAligned(s64 size, s64 align);
	void *Resize(void *mem, s64 newSize);
	void Deallocate(void *mem);
	void Clear();
	void Free();
};

struct NullAllocator *NullAllocator();

}
