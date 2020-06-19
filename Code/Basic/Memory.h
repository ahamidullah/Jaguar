#pragma once

#if defined(__linux__)
	#include "Linux/Memory.h"
#else
	#error Unsupported platform.
#endif

void InitializeMemory();

IntegerPointer AlignAddress(IntegerPointer address, s64 alignment);
void *AlignPointer(void *address, s64 alignment);
void SetMemory(void *destination, s64 byteCount, s8 setTo);
void CopyMemory(const void *source, void *destination, s64 byteCount);
void MoveMemory(void *source, void *destination, s64 byteCount);

void *AllocateMemory(s64 size);
void *AllocateAlignedMemory(s64 size, s64 alignment);
void *ResizeMemory(void *memory, s64 newSize);
void FreeMemory(void *memory);

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

void PushContextAllocator(AllocatorInterface allocator);
void PopContextAllocator(AllocatorInterface allocator);

extern AllocatorInterface globalHeapAllocator;
extern THREAD_LOCAL AllocatorInterface contextAllocator;

#include "Array.h"

struct AllocatorBlockList
{
	s64 blockSize;
	AllocatorInterface blockAllocator;
	Array<u8 *> usedBlocks;
	Array<u8 *> unusedBlocks;
	u8 *frontier;
	u8 *endOfCurrentBlock;
};

struct PoolAllocator
{
	AllocatorBlockList blockList;
	s64 defaultAlignment;
};

PoolAllocator NewPoolAllocator(s64 blockSize, s64 blockCount, AllocatorInterface blockAlloc, AllocatorInterface arrayAlloc);
AllocatorInterface NewPoolAllocatorInterface(PoolAllocator *pool);
void *AllocatePoolMemory(void *pool, s64 size);
void *AllocateAlignedPoolMemory(void *pool, s64 size, s64 alignment);
void *ResizePoolMemory(void *pool, void *memory, s64 newSize);
void FreePoolMemory(void *pool, void *memory);
void ClearPoolAllocator(void *pool);
void FreePoolAllocator(void *pool);

struct SlotAllocator
{
	AllocatorBlockList blockList;
	s64 slotSize;
	s64 slotAlignment;
	Array<void *> freeSlots;
};

SlotAllocator NewSlotAllocator(s64 slotSize, s64 slotAlignment, s64 slotCount, s64 slotsPerBlock, AllocatorInterface blockAlloc, AllocatorInterface arrayAlloc);
AllocatorInterface NewSlotAllocatorInterface(SlotAllocator *slots);
void *AllocateSlotMemory(void *slots, s64 size);
void *AllocateAlignedSlotMemory(void *slots, s64 size, s64 alignment);
void *ResizeSlotMemory(void *slots, void *memory, s64 newSize);
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
	AllocatorBlockList blockList;
	s64 defaultAlignment;
	Array<HeapAllocationHeader *> freeAllocations;
};

HeapAllocator NewHeapAllocator(s64 blockSize, s64 blockCount, AllocatorInterface blockAlloc, AllocatorInterface arrayAlloc);
AllocatorInterface NewHeapAllocatorInterface(HeapAllocator *heap);
void *AllocateHeapMemory(void *heap, s64 size);
void *AllocateAlignedHeapMemory(void *heap, s64 size, s64 alignment);
void *ResizeHeapMemory(void *heap, void *memory, s64 newSize);
void FreeHeapMemory(void *heap, void *memory);
void ClearHeapAllocator(void *heap);
void FreeHeapAllocator(void *heap);
