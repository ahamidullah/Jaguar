#pragma once

typedef void *(*AllocateMemoryProcedure)(void *allocator, s64 size);
typedef void *(*AllocateAlignedMemoryProcedure)(void *allocator, s64 size, s64 alignment);
typedef void *(*ResizeMemoryProcedure)(void *allocator, void *memory, s64 newSize);
typedef void (*FreeMemoryProcedure)(void *allocator, void *memory);
typedef void (*ResetAllocatorProcedure)(void *allocator);
typedef void (*DestroyAllocatorProcedure)(void *allocator);

struct MemoryAllocatorInterface
{
	void *data;
	AllocateMemoryProcedure allocate;
	AllocateAlignedMemoryProcedure allocateAligned;
	ResizeMemoryProcedure resize;
	FreeMemoryProcedure free;
	ResetAllocatorProcedure reset;
	DestroyAllocatorProcedure destroy;
};

void *AllocateMemory(MemoryAllocatorInterface allocator, s64 size);
void *AllocateAlignedMemory(MemoryAllocatorInterface allocator, s64 size, s64 alignment);
void *ResizeMemory(MemoryAllocatorInterface allocator, void *memory, s64 newSize);
void FreeMemory(MemoryAllocatorInterface allocator, void *memory);
void ResetAllocator(MemoryAllocatorInterface allocator);
void DestroyAllocator(MemoryAllocatorInterface allocator);
