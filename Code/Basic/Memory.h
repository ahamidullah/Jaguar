#pragma once

#if __linux__
	#include "Linux/Memory.h"
#else
	#error Unsupported platform.
#endif
#include "Allocator.h"
#include "Thread.h"
#include "Array.h"
#include "Spinlock.h"

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
	void Clear();
	void Free();
};

struct HeapAllocationHeader
{
	s64 size;
	s64 alignment;
};

struct HeapAllocator : Allocator
{
	AllocatorBlocks blocks;
	Array<HeapAllocationHeader *> free;

	void *Allocate(s64 size);
	void *AllocateAligned(s64 size, s64 align);
	void *Resize(void *mem, s64 size);
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
	void *Resize(void *mem, s64 size);
	void Deallocate(void *mem);
	void Clear();
	void Free();
};

GlobalHeapAllocator *GlobalHeap();

struct PoolAllocator : Allocator
{
	AllocatorBlocks blocks;

	void *Allocate(s64 size);
	void *AllocateAligned(s64 size, s64 align);
	void *Resize(void *mem, s64 size);
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
	void *Resize(void *mem, s64 size);
	void Deallocate(void *mem);
	void Clear();
	void Free();
};

SlotAllocator NewSlotAllocator(s64 slotSize, s64 slotAlignment, s64 slotCount, s64 slotsPerBlock, Allocator *blockAlloc, Allocator *arrayAlloc);

void *AllocateMemory(s64 size);
void *AllocateAlignedMemory(s64 size, s64 align);
void *ResizeMemory(void *mem, s64 size);
void DeallocateMemory(void *mem);
IntegerPointer AlignAddress(IntegerPointer addr, s64 align);
void *AlignPointer(void *addr, s64 align);
//void SetMemory(void *dst, s64 n, s8 to);
//void CopyMemory(const void *src, void *dst, s64 n);
//void MoveMemory(void *src, void *dst, s64 n);
void PushContextAllocator(Allocator *a);
void PopContextAllocator();
Allocator *ContextAllocator();
