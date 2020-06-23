#pragma once

#if defined(__linux__)
	#include "Linux/Memory.h"
#else
	#error Unsupported platform.
#endif

#include "AllocatorInterface.h"
#include "Thread.h"
#include "Array.h"

void InitializeMemory();

IntegerPointer AlignAddress(IntegerPointer addr, s64 align);
void *AlignPointer(void *addr, s64 align);
void SetMemory(void *dst, s64 n, s8 setTo);
void CopyMemory(const void *src, void *dst, s64 n);
void MoveMemory(void *src, void *dst, s64 n);

void *AllocateMemory(s64 size);
void *AllocateAlignedMemory(s64 size, s64 align);
void *ResizeMemory(void *mem, s64 size);
void FreeMemory(void *mem);

void PushContextAllocator(AllocatorInterface a);
void PopContextAllocator();

AllocatorInterface GlobalHeapAllocator();
AllocatorInterface ContextAllocator();

struct StackAllocator
{
	AllocatorInterface allocator;
	s64 size;
	s64 defaultAlignment;
	u8 *memory;
	u8 *top;
};

StackAllocator NewStackAllocator(s64 size, AllocatorInterface a);
StackAllocator NewStackAllocatorIn(s64 size, void *mem);
AllocatorInterface NewStackAllocatorInterface(StackAllocator *stack);
void *AllocateStackMemory(void *stack, s64 size);
void *AllocateAlignedStackMemory(void *stack, s64 size, s64 align);
void *ResizeStackMemory(void *stack, void *memory, s64 size);
void FreeStackMemory(void *stack, void *memory);
void ClearStackAllocator(void *stack);
void FreeStackAllocator(void *stack);

struct AllocatorBlocks
{
	s64 blockSize;
	AllocatorInterface allocator;
	Array<u8 *> used;
	Array<u8 *> unused;
	u8 *frontier;
	u8 *end;
};

struct PoolAllocator
{
	AllocatorBlocks blocks;
	s64 defaultAlignment;
};

PoolAllocator NewPoolAllocator(s64 blockSize, s64 blockCount, AllocatorInterface blockAlloc, AllocatorInterface arrayAlloc);
AllocatorInterface NewPoolAllocatorInterface(PoolAllocator *pool);
void *AllocatePoolMemory(void *pool, s64 size);
void *AllocateAlignedPoolMemory(void *pool, s64 size, s64 alignment);
void *ResizePoolMemory(void *pool, void *memory, s64 size);
void FreePoolMemory(void *pool, void *memory);
void ClearPoolAllocator(void *pool);
void FreePoolAllocator(void *pool);

struct SlotAllocator
{
	AllocatorBlocks blocks;
	s64 slotSize;
	s64 slotAlignment;
	Array<void *> freeSlots;
};

SlotAllocator NewSlotAllocator(s64 slotSize, s64 slotAlignment, s64 slotCount, s64 slotsPerBlock, AllocatorInterface blockAlloc, AllocatorInterface arrayAlloc);
AllocatorInterface NewSlotAllocatorInterface(SlotAllocator *slots);
void *AllocateSlotMemory(void *slots, s64 size);
void *AllocateAlignedSlotMemory(void *slots, s64 size, s64 alignment);
void *ResizeSlotMemory(void *slots, void *memory, s64 size);
void FreeSlotMemory(void *slots, void *memory);
void ClearSlotAllocator(void *slots);
void FreeSlotAllocator(void *slots);

struct HeapAllocationHeader
{
	s64 size;
	s64 alignment;
};

struct HeapAllocator
{
	AllocatorBlocks blocks;
	s64 defaultAlignment;
	Array<HeapAllocationHeader *> free;
};

HeapAllocator NewHeapAllocator(s64 blockSize, s64 blockCount, AllocatorInterface blockAlloc, AllocatorInterface arrayAlloc);
AllocatorInterface NewHeapAllocatorInterface(HeapAllocator *heap);
void *AllocateHeapMemory(void *heap, s64 size);
void *AllocateAlignedHeapMemory(void *heap, s64 size, s64 alignment);
void *ResizeHeapMemory(void *heap, void *memory, s64 size);
void FreeHeapMemory(void *heap, void *memory);
void ClearHeapAllocator(void *heap);
void FreeHeapAllocator(void *heap);
