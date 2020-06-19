#pragma once

#include "Code/Common.h"

typedef void *(*AllocateMemoryProcedure)(void *allocator, s64 size);
typedef void *(*AllocateAlignedMemoryProcedure)(void *allocator, s64 size, s64 alignment);
typedef void *(*ResizeMemoryProcedure)(void *allocator, void *memory, s64 newSize);
typedef void (*FreeMemoryProcedure)(void *allocator, void *memory);
typedef void (*ClearAllocatorProcedure)(void *allocator);
typedef void (*FreeAllocatorProcedure)(void *allocator);

struct AllocatorInterface
{
	void *data;
	AllocateMemoryProcedure allocateMemory;
	AllocateAlignedMemoryProcedure allocateAlignedMemory;
	ResizeMemoryProcedure resizeMemory;
	FreeMemoryProcedure freeMemory;
	ClearAllocatorProcedure clearAllocator;
	FreeAllocatorProcedure freeAllocator;
};
