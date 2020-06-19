#include "Memory.h"
#include "CPU.h"
#include "Log.h"
#include "Process.h"

const auto GLOBAL_HEAP_BLOCK_SIZE = MegabytesToBytes(64);
const auto GLOBAL_HEAP_INITIAL_BLOCK_COUNT = 32;

auto globalHeapAllocatorData = HeapAllocator{};
auto globalHeapAllocator = AllocatorInterface{};
auto baseContextAllocator = AllocatorInterface{};

THREAD_LOCAL auto contextAllocatorStack = Array<AllocatorInterface>{};
THREAD_LOCAL auto isContextAllocatorSet = false;
THREAD_LOCAL auto contextAllocator = AllocatorInterface{};

void *AllocateGlobalHeapBlock(void *, s64 size)
{
	Assert(size == GLOBAL_HEAP_BLOCK_SIZE);
	Assert(size % GetCPUPageSize() == 0);
	return AllocatePlatformMemory(size);
}

void FreeGlobalHeapBlock(void *, void *memory)
{
	FreePlatformMemory(memory, GLOBAL_HEAP_BLOCK_SIZE);
}

void *AllocateGlobalHeapArray(void *, s64 size)
{
	// In the interest of keeping this code simple, we're gonna assume that no alignment of the
	// data is necessary.  This should be fine since this allocator is only ever used to
	// allocate arrays of pointers for the global heap allocator.
	Assert(alignof(u8 *) % alignof(s64) == 0);
	size = AlignAddress(size + sizeof(size), GetCPUPageSize());
	auto allocation = AllocatePlatformMemory(size);
	*(s64 *)allocation = size;
	return (u8 *)allocation + sizeof(size);
}

void FreeGlobalHeapArray(void *, void *memory)
{
	auto size = *(s64 *)((u8 *)memory - sizeof(s64));
	FreePlatformMemory(memory, size);
}

void *ResizeGlobalHeapArray(void *, void *memory, s64 size)
{
	FreeGlobalHeapArray(NULL, memory);
	return AllocateGlobalHeapArray(NULL, size);
}

void InitializeMemory()
{
	auto globalHeapBlockAllocator = AllocatorInterface
	{
		.allocateMemory = AllocateGlobalHeapBlock,
		.freeMemory = FreeGlobalHeapBlock,
	};
	auto globalHeapArrayAllocator = AllocatorInterface
	{
		.allocateMemory = AllocateGlobalHeapArray,
		.resizeMemory = ResizeGlobalHeapArray,
		.freeMemory = FreeGlobalHeapArray,
	};
	globalHeapAllocatorData = NewHeapAllocator(GLOBAL_HEAP_BLOCK_SIZE, GLOBAL_HEAP_INITIAL_BLOCK_COUNT, globalHeapBlockAllocator, globalHeapArrayAllocator);
	globalHeapAllocator = NewHeapAllocatorInterface(&globalHeapAllocatorData);
	baseContextAllocator = globalHeapAllocator;
}

void PushContextAllocator(AllocatorInterface a)
{
	AppendToArray(&contextAllocatorStack, a);
	contextAllocator = contextAllocatorStack[contextAllocatorStack.count - 1];
}

void PopContextAllocator()
{
	if (contextAllocatorStack.count == 0)
	{
		LogPrint(ERROR_LOG, "Tried to pop empty context stack.\n");
		return;
	}
	ResizeArray(&contextAllocatorStack, contextAllocatorStack.count - 1);
	if (contextAllocatorStack.count == 0)
	{
		contextAllocator = baseContextAllocator;
	}
	else
	{
		contextAllocator = contextAllocatorStack[contextAllocatorStack.count - 1];
	}
}

void *AllocateMemory(s64 size)
{
	if (!isContextAllocatorSet)
	{
		contextAllocator = baseContextAllocator;
		isContextAllocatorSet = true;
	}
	return contextAllocator.allocateMemory(contextAllocator.data, size);
}

void *AllocateAlignedMemory(s64 size, s64 alignment)
{
	if (!isContextAllocatorSet)
	{
		contextAllocator = baseContextAllocator;
		isContextAllocatorSet = true;
	}
	return contextAllocator.allocateAlignedMemory(contextAllocator.data, size, alignment);
}

void *ResizeMemory(void *memory, s64 size)
{
	if (!isContextAllocatorSet)
	{
		contextAllocator = baseContextAllocator;
		isContextAllocatorSet = true;
	}
	return contextAllocator.resizeMemory(contextAllocator.data, memory, size);
}

void FreeMemory(void *memory)
{
	if (!isContextAllocatorSet)
	{
		contextAllocator = baseContextAllocator;
		isContextAllocatorSet = true;
	}
	contextAllocator.freeMemory(contextAllocator.data, memory);
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
	return (void *)AlignAddress((IntegerPointer)address, alignment);
}

void SetMemory(void *destination, s64 byteCount, s8 setTo)
{
	auto d = (s8 *)destination;
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

AllocatorBlockList NewAllocatorBlockList(s64 blockSize, s64 blockCount, AllocatorInterface blockAlloc, AllocatorInterface arrayAlloc)
{
	auto bl = AllocatorBlockList
	{
		.blockSize = blockSize,
		.blockAllocator = blockAlloc,
		.usedBlocks = NewArrayWithCapacityIn<u8 *>(0, blockCount, arrayAlloc),
		.unusedBlocks = NewArrayWithCapacityIn<u8 *>(0, blockCount, arrayAlloc),
	};
	for (auto i = 0; i < blockCount; i++)
	{
		AppendToArray(&bl.unusedBlocks, (u8 *)blockAlloc.allocateMemory(blockAlloc.data, blockSize));
	}
	return bl;
}

void AllocateBlockInList(AllocatorBlockList *bl)
{
	Assert(bl->blockSize > 0);
	if (bl->unusedBlocks.count > 0)
	{
		AppendToArray(&bl->usedBlocks, bl->unusedBlocks[bl->unusedBlocks.count - 1]);
		ResizeArray(&bl->unusedBlocks, bl->unusedBlocks.count - 1);
	}
	else
	{
		AppendToArray(&bl->usedBlocks, (u8 *)bl->blockAllocator.allocateMemory(bl->blockAllocator.data, bl->blockSize));
	}
	bl->frontier = bl->usedBlocks[bl->usedBlocks.count - 1];
	bl->endOfCurrentBlock = bl->frontier + bl->blockSize;
};

void ResetAllocatorBlockList(AllocatorBlockList *bl)
{
	Assert(bl->blockSize > 0);
	if (bl->frontier)
	{
		AppendCopyToArray(&bl->unusedBlocks, bl->usedBlocks);
		ResizeArray(&bl->usedBlocks, 0);
		bl->frontier = NULL;
	}
}

void FreeAllocatorBlockList(AllocatorBlockList *bl)
{
	// @TODO
}

#define DEFAULT_MEMORY_ALIGNMENT 16

PoolAllocator NewPoolAllocator(s64 blockSize, s64 blockCount, AllocatorInterface blockAlloc, AllocatorInterface arrayAlloc)
{
	return
	{
		.blockList = NewAllocatorBlockList(blockSize, blockCount, blockAlloc, arrayAlloc),
		.defaultAlignment = DEFAULT_MEMORY_ALIGNMENT,
	};
}

AllocatorInterface NewPoolAllocatorInterface(PoolAllocator *pool)
{
	return
	{
		.data = pool,
		.allocateMemory = AllocatePoolMemory,
		.allocateAlignedMemory = AllocateAlignedPoolMemory,
		.resizeMemory = ResizePoolMemory,
		.freeMemory = FreePoolMemory,
		.clearAllocator = ClearPoolAllocator,
		.freeAllocator = FreePoolAllocator,
	};
}

void *AllocatePoolMemory(void *pool, s64 size)
{
	auto p = (PoolAllocator *)pool;
	Assert(p->defaultAlignment > 0);
	return AllocateAlignedPoolMemory(p, size, p->defaultAlignment);
}

void *AllocateAlignedPoolMemory(void *pool, s64 size, s64 alignment)
{
	auto p = (PoolAllocator *)pool;
	Assert(alignment > 0);
	Assert(size + (alignment - 1) <= p->blockList.blockSize);
	if (!p->blockList.frontier || (u8 *)AlignPointer(p->blockList.frontier, alignment) + size > p->blockList.endOfCurrentBlock)
	{
		AllocateBlockInList(&p->blockList);
	}
	p->blockList.frontier = (u8 *)AlignPointer(p->blockList.frontier, alignment);
	auto result = p->blockList.frontier;
	p->blockList.frontier += size;
	return result;
}

void *ResizePoolMemory(void *pool, void *memory, s64 size)
{
	return AllocatePoolMemory(pool, size);
}

void FreePoolMemory(void *pool, void *memory)
{
}

void ClearPoolAllocator(void *pool)
{
	auto p = (PoolAllocator *)pool;
	ResetAllocatorBlockList(&p->blockList);
}

void FreePoolAllocator(void *poolPointer)
{
	// @TODO
}

SlotAllocator NewSlotAllocator(s64 slotSize, s64 slotAlignment, s64 slotCount, s64 slotsPerBlock, AllocatorInterface blockAlloc, AllocatorInterface arrayAlloc)
{
	auto blockCount = (slotCount + (slotsPerBlock - 1)) / slotsPerBlock; // Divide and round up.
	return
	{
		.blockList = NewAllocatorBlockList(slotsPerBlock * slotSize, blockCount, blockAlloc, arrayAlloc),
	 	.slotSize = slotSize,
	 	.slotAlignment = slotAlignment,
	 	.freeSlots = NewArrayIn<void *>(0, arrayAlloc),
	};
}

AllocatorInterface NewSlotAllocatorInterface(SlotAllocator *s)
{
	return
	{
		.data = s,
		.allocateMemory = AllocateSlotMemory,
		.allocateAlignedMemory = AllocateAlignedSlotMemory,
		.resizeMemory = ResizeSlotMemory,
		.freeMemory = FreeSlotMemory,
		.clearAllocator = ClearSlotAllocator,
		.freeAllocator = FreeSlotAllocator,
	};
}

void *AllocateSlotMemory(void *slots, s64 size)
{
	auto s = (SlotAllocator *)slots;
	Assert(size == s->slotSize);
	if (s->freeSlots.count > 0)
	{
		auto result = s->freeSlots[s->freeSlots.count - 1];
		ResizeArray(&s->freeSlots, s->freeSlots.count - 1);
		return result;
	}
	if (!s->blockList.frontier || s->blockList.frontier + s->slotSize > s->blockList.endOfCurrentBlock)
	{
		AllocateBlockInList(&s->blockList);
		s->blockList.frontier = (u8 *)AlignPointer(s->blockList.frontier, s->slotAlignment);
	}
	auto result = s->blockList.frontier;
	s->blockList.frontier += size;
	return result;
}

void *AllocateAlignedSlotMemory(void *slots, s64 size, s64 alignment)
{
	// Allocating aligned memory from a bucket allocator does not make sense.
	// Just return a regular allocation.
	return AllocateSlotMemory(slots, size);
}

void *ResizeSlotMemory(void *slots, void *memory, s64 size)
{
	Abort("Attempted to resize a slot memory allocation.\n");
	return NULL;
}

void FreeSlotMemory(void *slots, void *memory)
{
	auto s = (SlotAllocator *)slots;
	AppendToArray(&s->freeSlots, memory);
}

void ClearSlotAllocator(void *slots)
{
	auto s = (SlotAllocator *)slots;
	ResetAllocatorBlockList(&s->blockList);
	ResizeArray(&s->freeSlots, 0);
}

void FreeSlotAllocator(void *slots)
{
	// @TODO
	auto s = (SlotAllocator *)slots;
	ResizeArray(&s->freeSlots, 0);
}

HeapAllocator NewHeapAllocator(s64 blockSize, s64 blockCount, AllocatorInterface blockAlloc, AllocatorInterface arrayAlloc)
{
	return
	{
		.blockList = NewAllocatorBlockList(blockSize, blockCount, blockAlloc, arrayAlloc),
		.defaultAlignment = DEFAULT_MEMORY_ALIGNMENT,
		.freeAllocations = NewArrayIn<HeapAllocationHeader *>(0, arrayAlloc),
	};
}

AllocatorInterface NewHeapAllocatorInterface(HeapAllocator *heap)
{
	return
	{
		.data = heap,
		.allocateMemory = AllocateHeapMemory,
		.allocateAlignedMemory = AllocateAlignedHeapMemory,
		.resizeMemory = ResizeHeapMemory,
		.freeMemory = FreeHeapMemory,
		.clearAllocator = ClearHeapAllocator,
		.freeAllocator = FreeHeapAllocator,
	};
}

void *AllocateHeapMemory(void *heap, s64 size)
{
	auto h = (HeapAllocator *)heap;
	return AllocateAlignedHeapMemory(h, size, h->defaultAlignment);
}

void *AllocateAlignedHeapMemory(void *heap, s64 size, s64 alignment)
{
	auto h = (HeapAllocator *)heap;

	auto maximumAllocationSize = size + sizeof(HeapAllocationHeader) + (alignof(HeapAllocationHeader) - 1) + alignment;
	Assert(maximumAllocationSize <= h->blockList.blockSize);
	if (!h->blockList.frontier || h->blockList.frontier + maximumAllocationSize > h->blockList.endOfCurrentBlock)
	{
		AllocateBlockInList(&h->blockList);
	}

	auto header = (HeapAllocationHeader *)AlignPointer(h->blockList.frontier, alignof(HeapAllocationHeader));
	auto data = (u8 *)header + sizeof(HeapAllocationHeader);
	if ((IntegerPointer)data % alignment == 0)
	{
		// Make room to store number of bytes between the end of the header and the start of the data.
		// We need this when we go to free the pointer.
		data += alignment;
	}
	else
	{
		data = (u8 *)AlignPointer(data, alignment);
	}

	h->blockList.frontier = data + size;
	Assert(h->blockList.frontier < h->blockList.endOfCurrentBlock);

	header->size = size;
	header->alignment = alignment;

	auto bytesBetweenEndOfHeaderAndStartOfData = data - ((u8 *)header + sizeof(HeapAllocationHeader));
	Assert(bytesBetweenEndOfHeaderAndStartOfData < U8_MAX);
	*(data - 1) = bytesBetweenEndOfHeaderAndStartOfData;

	return data;
}

void *ResizeHeapMemory(void *heap, void *memory, s64 size)
{
	// @TODO
	FreeHeapMemory(heap, memory);
	return AllocateHeapMemory(heap, size);
}

void FreeHeapMemory(void *heap, void *memory)
{
	auto h = (HeapAllocator *)heap;
	auto bytesBetweenEndOfHeaderAndStartOfData = *((u8 *)memory - 1);
	auto header = (HeapAllocationHeader *)((u8 *)memory - bytesBetweenEndOfHeaderAndStartOfData);
	AppendToArray(&h->freeAllocations, header);
}

void ClearHeapAllocator(void *heap)
{
	auto h = (HeapAllocator *)heap;
	ResetAllocatorBlockList(&h->blockList);
	ResizeArray(&h->freeAllocations, 0);
}

void FreeHeapAllocator(void *heap)
{
	auto h = (HeapAllocator *)heap;
	// @TODO
	ResizeArray(&h->freeAllocations, 0);
}
