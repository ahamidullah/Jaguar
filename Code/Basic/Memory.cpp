#include "Memory.h"

void *AllocateMemory(s64 size)
{
	return AllocateMemory(&context.allocator, size);
}

void *AllocateAlignedMemory(s64 size, s64 alignment)
{
	return AllocateAlignedMemory(&context.allocator, size, alignment)
}

void *ResizeMemory(void *memory, s64 newSize)
{
	return ResizeMemory(context.allocator, memory, newSize);
}

void FreeMemory(void *memory)
{
	FreeMemory(&context.allocator);
}

IntegerPointer AlignAddress(IntegerPointer address, s64 alignment)
{
	// Code from Game Engine Architecture (2018).
	Assert(alignment > 0);
	auto mask = alignment - 1;
	Assert((alignment & mask) == 0); // Power of 2.
	return (address + mask) & ~mask;
}

void *AlignPointer(void *address, s64 alignment)
{
	return (void *)AlignAddress((IntegerPointer)address);
}

void SetMemory(void *destination, s8 setTo, s64 byteCount)
{
	auto d = (char *)destination;
	for (auto i = 0; i < byteCount; ++i)
	{
		d[i] = setTo;
	}
}

void CopyMemory(const void *source, void *destination, s64 byteCount)
{
	memcpy(destination, source, byteCount); // @TODO
}

// Only legal if source and destination are in the same array.
void MoveMemory(void *source, void *destination, s64 byteCount)
{
	auto s = (char *)source;
	auto d = (char *)destination;
	if (s < d)
	{
		for (s += byteCount, d += byteCount; byteCount; --byteCount)
		{
			*--d = *--s;
		}
	}
	else
	{
		while (byteCount--)
		{
			*d++ = *s++;
		}
	}
}

//
// Allocator stack
//

THREAD_LOCAL auto allocatorStack = Array<MemoryAllocatorInterface>{};
THREAD_LOCAL auto activeAllocator = MemoryAllocatorInterface{};
auto baseAllocator = MemoryAllocatorInterface{};

void PushMemoryAllocator(MemoryAllocatorInterface allocator)
{
	AppendToArray(&allocatorStack, context);
	activeContext = allocatorStack[allocatorStack.count - 1];
}

void PopMemoryAllocator()
{
	if (allocatorStack.count == 0)
	{
		LogPrint(ERROR_LOG, "Tried to pop empty context stack.\n");
		return;
	}
	ResizeArray(&allocatorStack, allocatorStack.count - 1);
	if (allocatorStack.count == 0)
	{
		activeAllocator = defaultContext;
	}
	else
	{
		activeAllocator = allocatorStack[allocatorStack.count - 1];
	}
}

const auto GLOBAL_HEAP_BLOCK_SIZE = MegabytesToBytes(64);
const auto GLOBAL_HEAP_INITIAL_BLOCK_COUNT = 32;
auto globalHeapAllocator = MemoryAllocatorInterface{};

void InitializeMemory()
{
	auto globalHeapBlockAllocator = MemoryAllocatorInterface
	{
		.data = NULL,
		.allocate = [](void *, s64 size)
		{
			Assert(size == GLOBAL_HEAP_BLOCK_SIZE);
			Assert(size % GetPageSize() == 0);
			return AllocatePlatformMemory(size);
		},
		.allocateAligned = NULL,
		.resize = NULL,
		.free = [](void *, void *memory)
		{
			FreePlatfromMemory(memory, GLOBAL_HEAP_BLOCK_SIZE);
		}
		.reset = NULL,
		.destroy = NULL,
	};
	auto globalHeapArrayAllocator = MemoryAllocatorInterface
	{
		.data = NULL,
		.allocate = [](void *, s64 size) -> void *
		{
			// In the interest of keeping this code simple, we're gonna assume that no alignment of the
			// data is necessary.  This should be fine since this allocator is only ever used to
			// allocate arrays of pointers for the global heap allocator.
			Assert(alignof(u8 *) % alignof(size) == 0);
			size = AlignAddress(size + sizeof(size), GetPageSize());
			auto allocation = AllocatePlatformMemory(size);
			*(s64 *)allocation = size;
			return (u8 *)allocation + sizeof(size);
		},
		.allocateAligned = NULL,
		.free = [](void *, void *memory)
		{
			auto size = *(s64 *)((u8 *)memory - sizeof(s64));
			FreePlatfromMemory(memory, size);
		},
		.resize = [](void *, void *memory, s64 newSize)
		{
			free(NULL, memory);
			return allocate(NULL, newSize);
		},
		.reset = NULL,
		.destroy = NULL,
	};
	globalHeapAllocator = CreateHeapAllocatorInterface(CreateHeapAllocator(GLOBAL_HEAP_BLOCK_SIZE, GLOBAL_HEAP_INITIAL_BLOCK_COUNT, globalHeapBlockAllocator, globalHeapArrayAllocator));

	baseAllocator = &globalHeapAllocator;
}

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
