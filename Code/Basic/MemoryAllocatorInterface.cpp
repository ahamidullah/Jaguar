#include "MemoryAllocatorInterface.h"

void *AllocateMemory(MemoryAllocator allocator, s64 size)
{
	return allocator.allocate(allocator.data, size);
}

void *AllocateAlignedMemory(MemoryAllocator allocator, s64 size, s64 alignment)
{
	return allocator.allocateAligned(allocator.data, size, alignment);
}

void *ResizeMemory(MemoryAllocator allocator, void *memory, s64 newSize)
{
	return allocator.resize(allocator.data, memory, newSize);
}

void FreeMemory(MemoryAllocator allocator, void *memory)
{
	allocator.free(allocator.data, memory);
}

void ResetAllocator(MemoryAllocator allocator)
{
	allocator.reset(allocator.data);
}

void DestroyAllocator(MemoryAllocator allocator)
{
	allocator.destroy(allocator.data);
}
