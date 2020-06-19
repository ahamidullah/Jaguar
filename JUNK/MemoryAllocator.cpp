#include "MemoryAllocator.h"

//
// Block
//

MemoryBlockList CreateMemoryBlockList(s64 blockSize, s64 initialBlockCount, MemoryAllocatorInterface blockAllocator, MemoryAllocatorInterface arrayAllocator);
{
	auto blockList = MemoryBlockList
	{
		.blockSize = blockSize,
	};
	blockList.blockAllocator = blockAllocator;
	blockList.usedBlocks = CreateArray<u8 *>(0, initialBlockCount, arrayAllocator);
	blockList.unusedBlocks = CreateArray<u8 *>(0, initialBlockCount, arrayAllocator);
	for (auto i = 0; i < initialBlockCount; i++)
	{
		ArrayAppend(&blockList.unusedBlocks, AllocateMemory(blockAllocator, blockSize));
	}
	return blockList;
}

void AllocateNewMemoryBlockInList(MemoryBlockList *blockList)
{
	Assert(blockList->blockSize > 0);
	if (blockList->unusedBlocks.count > 0)
	{
		ArrayAppend(&blockList->usedBlocks, blockList->unusedBlocks[blockList->unusedBlocks.count - 1]);
		ResizeArray(&blockList.unusedBlocks, blockList->unusedBlocks.count - 1);
	}
	else
	{
		ArrayAppend(&blockList.usedBlocks, blockList->blockAllocator.allocate(blockList->blockAllocator->data, blockList->blockSize));
	}
	blockList->frontier = blockList->usedBlocks[blockList->usedBlocks.count - 1];
	blockList->endOfCurrentBlock = blockList->frontier + blockList->blockSize;
};

void ResetMemoryBlockList(MemoryBlockList *blockList)
{
	Assert(list->blockSize > 0);

	if (blockList->frontier)
	{
		ArrayAppend(&blockList->unusedBlocks, &blockList->usedBlocks);
		blockList->frontier = NULL;
	}
}

void DestoryMemoryBlockList(MemoryBlockList *blockList)
{
	// @TODO
}

#define DEFAULT_MEMORY_ALIGNMENT 16

//
// Pool
//

PoolAllocator CreatePoolAllocator(s64 blockSize, s64 initialBlockCount, MemoryAllocatorInterface blockAllocator, MemoryAllocatorInterface arrayAllocator)
{
	return
	{
		.blockList = CreateMemoryBlockList(blockSize, initialBlockCount, blockAllocator, arrayAllocator),
		.defaultAlignment = DEFAULT_MEMORY_ALIGNMENT,
	};
}

MemoryAllocatorInterface CreatePoolAllocatorInterface(PoolAllocator *pool)
{
	return
	{
		.data = pool,
		.allocate = AllocatePoolMemory,
		.allocateAligned = AllocateAlignedPoolMemory,
		.resize = ResizePoolMemory,
		.free = FreePoolMemory,
		.reset = ResetPoolAllocator,
		.destroy = DestroyPoolAllocator,
	};
}

void *AllocatePoolMemory(void *poolPointer, s64 size)
{
	auto pool = (PoolAllocator *)poolPointer;
	Assert(pool->defaultAlignment > 0);
	return AllocateAlignedPoolMemory(&pool->blockList, size, pool->defaultAlignment);
}

void *AllocateAlignedPoolMemory(void *poolPointer, s64 size, s64 alignment)
{
	auto pool = (PoolAllocator *)poolPointer;
	Assert(alignment > 0);
	Assert(size + (alignment - 1) <= pool->blockList.blockSize);
	if (!pool->blockList.frontier || AlignPointer(pool->blockList.frontier, alignment) + size > pool->blockList.endOfCurrentBlock)
	{
		AllocateNewMemoryBlockInList(&pool->blockList);
	}
	pool->blockList->frontier = AlignPointer(pool->blockList.frontier, alignment);
	auto result = pool->blockList.frontier;
	pool->blockList.frontier += size;
	return result;
}

void *ResizePoolMemory(void *poolPointer, void *memory, s64 newSize)
{
	return AllocatePoolMemory(poolPointer, newSize);
}

void FreePoolMemory(void *poolPointer, void *memory)
{
}

void ResetPoolAllocator(void *poolPointer)
{
	auto pool = (PoolAllocator *)poolPointer;
	ResetMemoryBlockList(&pool->blockList);
}

void DestroyPoolAllocator(void *poolPointer)
{
	// @TODO
}

//
// Slot
//

SlotAllocator CreateSlotAllocator(s64 slotSize, s64 slotAlignment, s64 initialSlotCount, s64 slotsPerBlock, MemoryAllocatorInterface blockAllocator, MemoryAllocatorInterface arrayAllocator)
{
	return
	{
		.blockList = CreateMemoryBlockList(slotsPerBlock * slotSize, DivideAndRoundUp(initialSlotCount, slotsPerBlock), blockAllocator, arrayAllocator),
	 	.slotSize = slotSize,
	 	.slotAlignment = slotAlignment,
	 	.freeSlots = CreateArray(0, arrayAllocator),
	};
}

MemoryAllocatorInterface CreateSlotAllocatorInterface(SlotAllocator *slots)
{
	return
	{
		.data = slots,
		.allocate = AllocateSlotMemory,
		.allocateAligned = AllocateAlignedSlotMemory,
		.resize = ResizeSlotMemory,
		.free = FreeSlotMemory,
		.reset = ResetSlotAllocator,
		.destroy = DestroySlotAllocator,
	};
}

void *AllocateSlotMemory(SlotAllocator *slots, s64 size)
{
	Assert(size == slots->slotSize);

	if (slots->freeSlots.count > 0)
	{
		auto result = slots->freeSlots[slots->freeSlots.count - 1];
		ResizeArray(&slots->freeSlots, slots->freeSlots.count - 1);
		return result;
	}
	if (!slots->blockList.frontier || slots->blockList.frontier + slots->slotSize > slots->blockList.endOfCurrentBlock)
	{
		AllocateNewMemoryBlockInList(&slots->blockList);
		blockList->frontier = AlignPointer(slots->blockList.frontier, slots->slotAlignment);
	}
	auto result = slots->blockList.frontier;
	slots->blockList.frontier += size;
	return result;
}

void *AllocateAlignedSlotMemory(SlotAllocator *slots, s64 size, s64 alignment)
{
	// Allocating aligned memory from a bucket allocator does not make sense.
	// Just return a regular allocation.
	return AllocatePoolMemory(&slots->pool, size);
}

void *ResizeSlotMemory(SlotAllocator *slots, void *memory, s64 newSize)
{
	Abort("Attempted to resize memory allocated from a SlotAllocator.\n");
}

void FreeSlotMemory(SlotAllocator *slots, void *memory)
{
	ArrayAppend(&slots->freeSlots, memory);
}

void ResetSlotAllocator(SlotAllocator *slots)
{
	ResetMemoryBlockList(&slots->blockList);
	ResizeArray(&slots->freeSlots, 0);
}

void DestroySlotAllocator(SlotAllocator *slots)
{
	// @TODO
	ResizeArray(&slots->freeSlots, 0);
}

//
// Heap
//

HeapAllocator CreateHeapAllocator(s64 blockSize, s64 initialBlockCount, MemoryAllocatorInterface blockAllocator, MemoryAllocatorInterface arrayAllocator)
{
	return
	{
		.blockList = CreateMemoryBlockList(blockSize, initialBlockCount, blockAllocator, arrayAllocator),
		.defaultAlignment = DEFAULT_MEMORY_ALIGNMENT,
		.freeAllocations = CreateArray(0, arrayAllocator),
	};
}

MemoryAllocatorInterface CreateHeapAllocatorInterface(HeapAllocator *heap)
{
	return
	{
		.data = buckets,
		.allocate = AllocateHeapMemory,
		.allocateAligned = AllocateAlignedHeapMemory,
		.resize = ResizeHeapMemory,
		.free = FreeHeapMemory,
		.reset = ResetHeapAllocator,
		.destroy = DestroyHeapAllocator,
	};
}

void *AllocateHeapMemory(HeapAllocator *heap, s64 size)
{
	AllocateAlignedHeapMemory(heap, size, heap->defaultAlignment);
}

void *AllocateAlignedHeapMemory(HeapAllocator *heap, s64 size, s64 alignment)
{
	auto maximumAllocationSize = size + sizeof(HeapAllocationHeader) + (alignof(HeapAllocationHeader) - 1) + alignment;
	Assert(maximumAllocationSize <= blockList->blockSize);
	if (!heap->blockList.frontier || heap->blockList.frontier + maximumAllocationSize > heap->blockList.endOfCurrentBlock)
	{
		AllocateNewMemoryBlockInList(&heap.blockList);
	}

	auto header (HeapAllocationHeader *)AlignPointer(heap->blockList.frontier, headerAlignment);
	auto data = (u8 *)*header + sizeof(HeapAllocationHeader);
	if ((IntegerPointer)data % dataAlignment == 0)
	{
		// Make room to store number of bytes between the end of the header and the start of the data.
		// We need this when we go to free the pointer.
		data += alignment;
	}
	else
	{
		data = AlignPointer(data, alignment);
	}

	heap->blockList.frontier = data + size;
	Assert(heap->blockList.frontier < heap->blockList.endOfCurrentBlock);

	header->size = size;
	header->alignment = alignment;

	auto bytesBetweenEndOfHeaderAndStartOfData = data - (header + headerSize);
	Assert(bytesBetweenEndOfHeaderAndStartOfData < U8_MAX);
	*(*data - 1) = bytesBetweenEndOfHeaderAndStartOfData;

	return data;
}

void *ResizeHeapMemory(void *heapPointer, void *memory, s64 newSize)
{
	// @TODO
	FreeHeapMemory(heapPointer, memory);
	return AllocateHeapMemory(heapPointer, newSize);
}

void FreeHeapMemory(HeapAllocator *heap, void *memory)
{
	auto bytesBetweenEndOfHeaderAndStartOfData = *((char *)memory - 1);
	auto header = (HeapAllocationHeader *)((char *)memory - bytesBetweenEndOfHeaderAndStartOfData);
	ArrayAppend(heap->freeAllocations, header);
}

void ResetHeapAllocator(HeapAllocator *heap)
{
	ResetMemoryBlockList(&heap->blockList);
	ResizeArray(&heap->freeAllocations, 0);
}

void DestroyHeapAllocator(HeapAllocator *heap)
{
	// @TODO
	ResizeArray(&heap->freeAllocations, 0);
}
