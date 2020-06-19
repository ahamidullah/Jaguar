#pragma once

#include "MemoryAllocatorInterface.h"
#include "Array.h"

struct MemoryBlockList
{
	s64 blockSize;
	MemoryAllocatorInterface blockAllocator;
	Array<u8 *> usedBlocks;
	Array<u8 *> unusedBlocks;
	u8 *frontier;
	u8 *endOfCurrentBlock;
};

struct PoolAllocator
{
	MemoryBlockList blockList;
	s64 defaultAlignment;
};

PoolAllocator CreatePoolAllocator(s64 blockSize, s64 initialBlockCount);
MemoryAllocatorInterface CreatePoolAllocatorInterface(PoolAllocator *pool);
void *AllocatePoolMemory(PoolAllocator *pool, s64 size);
void *AllocateAlignedPoolMemory(PoolAllocator *pool, s64 size, s64 alignment);
void *ResizePoolMemory(PoolAllocator *pool, void *memory, s64 newSize);
void FreePoolMemory(PoolAllocator *pool, void *memory);
void ResetPoolAllocator(PoolAllocator *pool);
void DestroyPoolAllocator(PoolAllocator *pool);

struct SlotAllocator
{
	MemoryBlockList blockList;
	s64 slotSize;
	s64 slotAlignment;
	Array<void *> freeSlots;
};

SlotAllocator CreateSlotAllocator(s64 slotSize, s64 initialSlotCount, s64 bucketCapacity);
MemoryAllocatorInterface CreateSlotAllocatorInterface(SlotAllocator *slots);
void *AllocateSlotMemory(SlotAllocator *slots, s64 size);
void *AllocateAlignedSlotMemory(SlotAllocator *slots, s64 size, s64 alignment);
void *ResizeSlotMemory(SlotAllocator *slots, void *memory, s64 newSize);
void FreeSlotMemory(SlotAllocator *buckets, void *memory);
void ResetSlotAllocator(SlotAllocator *buckets);
void DestroySlotAllocator(SlotAllocator *buckets);

struct HeapAllocationHeader
{
	s64 size;
	s64 alignment;
};

struct HeapAllocator
{
	MemoryBlockList blockList;
	s64 defaultAlignment;
	Array<HeapAllocationHeader *> freeAllocations;
};

HeapAllocator CreateHeapAllocator(s64 blockSize, s64 initialBlockCount, MemoryAllocatorInterface blockAllocator, MemoryAllocatorInterface arrayAllocator);
MemoryAllocatorInterface CreateHeapAllocatorInterface(HeapAllocator *heap);
void *AllocateHeapMemory(HeapAllocator *heap, s64 size);
void *AllocateAlignedHeapMemory(HeapAllocator *heap, s64 size, s64 alignment);
void *ResizeSlotMemory(SlotAllocator *slots, void *memory, s64 newSize);
void FreeHeapMemory(HeapAllocator *heap, void *memory);
void ResetHeapAllocator(HeapAllocator *heap);
void DestroyHeapAllocator(HeapAllocator *heap);
