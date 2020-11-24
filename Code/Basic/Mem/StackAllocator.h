#pragma once

#include "Allocator.h"
#include "AllocatorBlocks.h"

namespace Memory
{

struct StackAllocator : Allocator
{
	u8 *buffer;
	u8 *head;
	s64 size;

	void *Allocate(s64 size);
	void *AllocateWithHeader(s64 size, s64 align);
	void *AllocateAligned(s64 size, s64 align);
	void *Resize(void *mem, s64 newSize);
	void Deallocate(void *mem);
	void Clear();
	void Free();
};

StackAllocator NewStackAllocator(array::View<u8> mem);

}

