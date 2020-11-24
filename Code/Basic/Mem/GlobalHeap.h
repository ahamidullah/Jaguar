#pragma once

#include "HeapAllocator.h"
#include "StackAllocator.h"
#include "Basic/Thread.h"
#include "Basic/Container/Array.h"

namespace Memory
{

struct GlobalHeapAllocator : Allocator
{
	Spinlock lock;
	s64 lockThreadID;
	HeapAllocator heap;
	StackAllocator backup;
	array::Static<u8, 8 * Megabyte> backupBuffer;

	void *Allocate(s64 size);
	void *AllocateAligned(s64 size, s64 align);
	void *Resize(void *mem, s64 newSize);
	void Deallocate(void *mem);
	void Clear();
	void Free();
};

GlobalHeapAllocator *GlobalHeap();

}
