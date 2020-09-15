#pragma once

#if __linux__
	#include "Linux/Memory.h"
#else
	#error Unsupported platform.
#endif
#include "Allocator.h"
#include "Thread.h"
#include "Array.h"

void InitializeMemory();

struct AllocatorBlocks
{
	s64 blockSize;
	Allocator *allocator;
	Array<u8 *> used;
	Array<u8 *> unused;
	u8 *frontier;
	u8 *end;

	void NewBlock();
	void *Allocate(s64 size, s64 align);
	void *AllocateWithHeader(s64 size, s64 align);
	void Clear();
	void Free();
};

struct AllocationHeader
{
	s64 size;
	s64 alignment;
	void *startOfAllocation;
};

struct HeapAllocator : Allocator
{
	AllocatorBlocks blocks;
	Array<AllocationHeader *> free;

	void *Allocate(s64 size);
	void *AllocateAligned(s64 size, s64 align);
	void *Resize(void *mem, s64 newSize);
	void Deallocate(void *mem);
	void Clear();
	void Free();
};

HeapAllocator NewHeapAllocator(s64 blockSize, s64 blockCount, Allocator *blockAlloc, Allocator *arrayAlloc);

struct GlobalHeapAllocator : Allocator
{
	Spinlock lock;
	HeapAllocator heap;

	void *Allocate(s64 size);
	void *AllocateAligned(s64 size, s64 align);
	void *Resize(void *mem, s64 newSize);
	void Deallocate(void *mem);
	void Clear();
	void Free();
};

Allocator *GlobalAllocator();

struct PoolAllocator : Allocator
{
	AllocatorBlocks blocks;

	void *Allocate(s64 size);
	void *AllocateAligned(s64 size, s64 align);
	void *Resize(void *mem, s64 newSize);
	void Deallocate(void *mem);
	void Clear();
	void Free();
};

PoolAllocator NewPoolAllocator(s64 blockSize, s64 blockCount, Allocator *blockAlloc, Allocator *arrayAlloc);

struct SlotAllocator : Allocator
{
	AllocatorBlocks blocks;
	s64 slotSize;
	s64 slotAlignment;
	Array<void *> freeSlots;

	void *Allocate(s64 size);
	void *AllocateAligned(s64 size, s64 align);
	void *Resize(void *mem, s64 newSize);
	void Deallocate(void *mem);
	void Clear();
	void Free();
};

SlotAllocator NewSlotAllocator(s64 slotSize, s64 slotAlignment, s64 slotCount, s64 slotsPerBlock, Allocator *blockAlloc, Allocator *arrayAlloc);

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

StackAllocator NewStackAllocator(ArrayView<u8> mem);

void *AllocateMemory(s64 size);
void *AllocateAlignedMemory(s64 size, s64 align);
void *ResizeMemory(void *mem, s64 newSize);
void DeallocateMemory(void *mem);
IntegerPointer AlignAddress(IntegerPointer addr, s64 align);
void *AlignPointer(void *addr, s64 align);
void PushContextAllocator(Allocator *a);
void PopContextAllocator();
Allocator *ContextAllocator();
void SetContextAllocator(Allocator *a);
