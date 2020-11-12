#pragma once

#include "Allocator.h"
#include "AllocatorBlocks.h"
#include "AllocationHeader.h"

namespace Memory
{

struct HeapAllocator : Allocator
{
	BlockAllocator blocks;
	array::Array<AllocationHeader *> free;

	void *Allocate(s64 size);
	void *AllocateAligned(s64 size, s64 align);
	void *Resize(void *mem, s64 newSize);
	void Deallocate(void *mem);
	void Clear();
	void Free();
};

HeapAllocator NewHeapAllocator(s64 blockSize, s64 blockCount, Allocator *blockAlloc, Allocator *arrayAlloc);

}
