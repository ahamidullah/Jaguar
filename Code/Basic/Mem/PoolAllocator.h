#pragma once

#include "Allocator.h"
#include "AllocatorBlocks.h"

namespace mem
{

struct PoolAllocator : Allocator
{
	BlockAllocator blocks;

	void *Allocate(s64 size);
	void *AllocateAligned(s64 size, s64 align);
	void *Resize(void *mem, s64 newSize);
	void Deallocate(void *mem);
	void Clear();
	void Free();
};

PoolAllocator NewPoolAllocator(s64 blockSize, s64 blockCount, Allocator *blockAlloc, Allocator *arrayAlloc);

}
