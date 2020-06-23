#include "Memory.h"
#include "CPU.h"
#include "Log.h"
#include "Process.h"

const auto DefaultMemoryAlignment = 16;

const auto GLOBAL_HEAP_BLOCK_SIZE = MegabytesToBytes(64);
const auto GLOBAL_HEAP_INITIAL_BLOCK_COUNT = 32;

auto globalHeapAllocatorData = HeapAllocator{};
auto globalHeapAllocator = AllocatorInterface{};
auto baseContextAllocator = AllocatorInterface{};

THREAD_LOCAL auto contextAllocatorStack = Array<AllocatorInterface>{};
THREAD_LOCAL auto isContextAllocatorSet = false;
THREAD_LOCAL auto contextAllocator = AllocatorInterface{};

AllocatorInterface ContextAllocator()
{
	return contextAllocator;
}

void *AllocateGlobalHeapBlock(void *, s64 size)
{
	Assert(size == GLOBAL_HEAP_BLOCK_SIZE);
	Assert(size % CPUPageSize() == 0);
	return AllocatePlatformMemory(size);
}

void FreeGlobalHeapBlock(void *, void *mem)
{
	FreePlatformMemory(mem, GLOBAL_HEAP_BLOCK_SIZE);
}

void *AllocateGlobalHeapArray(void *, s64 size)
{
	// In the interest of keeping this code simple, we're gonna assume that no alignment of the
	// data is necessary.  This should be fine since this allocator is only ever used to
	// allocate arrays of pointers for the global heap allocator.
	Assert(alignof(u8 *) % alignof(s64) == 0);
	size = AlignAddress(size + sizeof(size), CPUPageSize());
	auto alloc = AllocatePlatformMemory(size);
	*(s64 *)alloc = size;
	return (u8 *)alloc + sizeof(size);
}

void FreeGlobalHeapArray(void *, void *mem)
{
	auto size = *(s64 *)((u8 *)mem - sizeof(s64));
	FreePlatformMemory(mem, size);
}

void *ResizeGlobalHeapArray(void *, void *mem, s64 size)
{
	FreeGlobalHeapArray(NULL, mem);
	return AllocateGlobalHeapArray(NULL, size);
}

void InitializeMemory()
{
	auto block = AllocatorInterface
	{
		.allocateMemory = AllocateGlobalHeapBlock,
		.freeMemory = FreeGlobalHeapBlock,
	};
	auto array = AllocatorInterface
	{
		.allocateMemory = AllocateGlobalHeapArray,
		.resizeMemory = ResizeGlobalHeapArray,
		.freeMemory = FreeGlobalHeapArray,
	};
	globalHeapAllocatorData = NewHeapAllocator(GLOBAL_HEAP_BLOCK_SIZE, GLOBAL_HEAP_INITIAL_BLOCK_COUNT, block, array);
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
		LogPrint(LogLevelError, "Memory", "Tried to pop empty context stack.\n");
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

void *AllocateAlignedMemory(s64 size, s64 align)
{
	if (!isContextAllocatorSet)
	{
		contextAllocator = baseContextAllocator;
		isContextAllocatorSet = true;
	}
	return contextAllocator.allocateAlignedMemory(contextAllocator.data, size, align);
}

void *ResizeMemory(void *mem, s64 size)
{
	if (!isContextAllocatorSet)
	{
		contextAllocator = baseContextAllocator;
		isContextAllocatorSet = true;
	}
	return contextAllocator.resizeMemory(contextAllocator.data, mem, size);
}

void FreeMemory(void *mem)
{
	if (!isContextAllocatorSet)
	{
		contextAllocator = baseContextAllocator;
		isContextAllocatorSet = true;
	}
	contextAllocator.freeMemory(contextAllocator.data, mem);
}

IntegerPointer AlignAddress(IntegerPointer addr, s64 align)
{
	// Code from Game Engine Architecture (2018).
	Assert(align > 0);
	auto mask = align - 1;
	Assert((align & mask) == 0); // Power of 2.
	return (addr + mask) & ~mask;
}

void *AlignPointer(void *addr, s64 align)
{
	return (void *)AlignAddress((IntegerPointer)addr, align);
}

void SetMemory(void *mem, s64 n, s8 to)
{
	auto m = (s8 *)mem;
	for (auto i = 0; i < n; ++i)
	{
		m[i] = to;
	}
}

void CopyMemory(const void *src, void *dst, s64 n)
{
	memcpy(dst, src, n); // @TODO
}

// Only legal if source and destination are in the same array.
void MoveMemory(void *src, void *dst, s64 n)
{
	auto s = (s8 *)src;
	auto d = (s8 *)dst;
	if (src < dst)
	{
		for (s += n, d += n; n; --n)
		{
			*--d = *--s;
		}
	}
	else
	{
		while (n--)
		{
			*d++ = *s++;
		}
	}
}

StackAllocator NewStackAllocator(s64 size, AllocatorInterface a)
{
	auto mem = (u8 *)a.allocateMemory(a.data, size);
	return
	{
		.allocator = a,
		.size = size,
		.defaultAlignment = DefaultMemoryAlignment,
		.memory = mem,
		.top = mem,
	};
}

StackAllocator NewStackAllocatorIn(s64 size, u8 *mem)
{
	return
	{
		.allocator =
		{
			.allocateMemory = [](void *, s64) -> void * { return NULL; },
			.allocateAlignedMemory = [](void *, s64, s64) -> void * { return NULL; },
			.resizeMemory = [](void *, void *, s64) -> void * { return NULL; },
			.freeMemory = [](void *, void *){},
			.clearAllocator = [](void *){},
			.freeAllocator = [](void *){},
		},
		.size = size,
		.defaultAlignment = DefaultMemoryAlignment,
		.memory = mem,
		.top = mem,
	};
}

AllocatorInterface NewStackAllocatorInterface(StackAllocator *stack)
{
	return
	{
		.data = stack,
		.allocateMemory = AllocateStackMemory,
		.allocateAlignedMemory = AllocateAlignedStackMemory,
		.resizeMemory = ResizeStackMemory,
		.freeMemory = FreeStackMemory,
		.clearAllocator = ClearStackAllocator,
		.freeAllocator = FreeStackAllocator,
	};
}

void *AllocateStackMemory(void *stack, s64 size)
{
	auto s = (StackAllocator *)stack;
	return AllocateAlignedStackMemory(s, size, s->defaultAlignment);
}

void *AllocateAlignedStackMemory(void *stack, s64 size, s64 align)
{
	auto s = (StackAllocator *)stack;
	auto r = (u8 *)AlignPointer(s->top, align);
	if (r + size > s->memory + s->size)
	{
		Abort("Stack allocator overflow.");
	}
	s->top = r + size;
	return r;
}

void *ResizeStackMemory(void *stack, void *mem, s64 size)
{
	return AllocateStackMemory(stack, size);
}

void FreeStackMemory(void *stack, void *mem)
{
}

void ClearStackAllocator(void *stack)
{
	auto s = (StackAllocator *)stack;
	s->top = s->memory;
}

void FreeStackAllocator(void *stack)
{
	auto s = (StackAllocator *)stack;
	s->allocator.freeMemory(s->allocator.data, s->memory);
}

AllocatorBlocks NewAllocatorBlocks(s64 blockSize, s64 blockCount, AllocatorInterface blockAlloc, AllocatorInterface arrayAlloc)
{
	auto b = AllocatorBlocks
	{
		.blockSize = blockSize,
		.allocator = blockAlloc,
		.used = NewArrayWithCapacityIn<u8 *>(0, blockCount, arrayAlloc),
		.unused = NewArrayWithCapacityIn<u8 *>(0, blockCount, arrayAlloc),
	};
	for (auto i = 0; i < blockCount; i++)
	{
		AppendToArray(&b.unused, (u8 *)blockAlloc.allocateMemory(blockAlloc.data, blockSize));
	}
	return b;
}

void AllocateBlock(AllocatorBlocks *b)
{
	Assert(b->blockSize > 0);
	if (b->unused.count > 0)
	{
		AppendToArray(&b->used, b->unused[b->unused.count - 1]);
		ResizeArray(&b->unused, b->unused.count - 1);
	}
	else
	{
		AppendToArray(&b->used, (u8 *)b->allocator.allocateMemory(b->allocator.data, b->blockSize));
	}
	b->frontier = b->used[b->used.count - 1];
	b->end = b->frontier + b->blockSize;
};

void ResetAllocatorBlocks(AllocatorBlocks *b)
{
	Assert(b->blockSize > 0);
	if (b->frontier)
	{
		AppendCopyToArray(&b->unused, b->used);
		ResizeArray(&b->used, 0);
		b->frontier = NULL;
	}
}

void FreeAllocatorBlocks(AllocatorBlocks *b)
{
	// @TODO
}

PoolAllocator NewPoolAllocator(s64 blockSize, s64 blockCount, AllocatorInterface blockAlloc, AllocatorInterface arrayAlloc)
{
	return
	{
		.blocks = NewAllocatorBlocks(blockSize, blockCount, blockAlloc, arrayAlloc),
		.defaultAlignment = DefaultMemoryAlignment,
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

void *AllocateAlignedPoolMemory(void *pool, s64 size, s64 align)
{
	auto p = (PoolAllocator *)pool;
	Assert(align > 0);
	Assert(size + (align - 1) <= p->blocks.blockSize);
	if (!p->blocks.frontier || (u8 *)AlignPointer(p->blocks.frontier, align) + size > p->blocks.end)
	{
		AllocateBlock(&p->blocks);
	}
	p->blocks.frontier = (u8 *)AlignPointer(p->blocks.frontier, align);
	auto result = p->blocks.frontier;
	p->blocks.frontier += size;
	return result;
}

void *ResizePoolMemory(void *pool, void *m, s64 size)
{
	return AllocatePoolMemory(pool, size);
}

void FreePoolMemory(void *pool, void *m)
{
}

void ClearPoolAllocator(void *pool)
{
	auto p = (PoolAllocator *)pool;
	ResetAllocatorBlocks(&p->blocks);
}

void FreePoolAllocator(void *pool)
{
	// @TODO
}

SlotAllocator NewSlotAllocator(s64 slotSize, s64 slotAlign, s64 slotCount, s64 slotsPerBlock, AllocatorInterface blockAlloc, AllocatorInterface arrayAlloc)
{
	auto blockCount = (slotCount + (slotsPerBlock - 1)) / slotsPerBlock; // Divide and round up.
	return
	{
		.blocks = NewAllocatorBlocks(slotsPerBlock * slotSize, blockCount, blockAlloc, arrayAlloc),
	 	.slotSize = slotSize,
	 	.slotAlignment = slotAlign,
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
	if (!s->blocks.frontier || s->blocks.frontier + s->slotSize > s->blocks.end)
	{
		AllocateBlock(&s->blocks);
		s->blocks.frontier = (u8 *)AlignPointer(s->blocks.frontier, s->slotAlignment);
	}
	auto result = s->blocks.frontier;
	s->blocks.frontier += size;
	return result;
}

void *AllocateAlignedSlotMemory(void *slots, s64 size, s64 align)
{
	// Allocating aligned memory from a bucket allocator does not make sense.
	// Just return a regular allocation.
	return AllocateSlotMemory(slots, size);
}

void *ResizeSlotMemory(void *slots, void *m, s64 size)
{
	Abort("Attempted to resize a slot memory allocation.\n");
	return NULL;
}

void FreeSlotMemory(void *slots, void *m)
{
	auto s = (SlotAllocator *)slots;
	AppendToArray(&s->freeSlots, m);
}

void ClearSlotAllocator(void *slots)
{
	auto s = (SlotAllocator *)slots;
	ResetAllocatorBlocks(&s->blocks);
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
		.blocks = NewAllocatorBlocks(blockSize, blockCount, blockAlloc, arrayAlloc),
		.defaultAlignment = DefaultMemoryAlignment,
		.free = NewArrayIn<HeapAllocationHeader *>(0, arrayAlloc),
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

void *AllocateAlignedHeapMemory(void *heap, s64 size, s64 align)
{
	auto h = (HeapAllocator *)heap;
	auto maxSize = size + sizeof(HeapAllocationHeader) + (alignof(HeapAllocationHeader) - 1) + align;
	Assert(maxSize <= h->blocks.blockSize);
	if (!h->blocks.frontier || h->blocks.frontier + maxSize > h->blocks.end)
	{
		AllocateBlock(&h->blocks);
	}
	auto header = (HeapAllocationHeader *)AlignPointer(h->blocks.frontier, alignof(HeapAllocationHeader));
	auto data = (u8 *)header + sizeof(HeapAllocationHeader);
	if ((IntegerPointer)data % align == 0)
	{
		// Make room to store number of bytes between the end of the header and the start of the data.
		// We need this when we go to free the pointer.
		data += align;
	}
	else
	{
		data = (u8 *)AlignPointer(data, align);
	}
	h->blocks.frontier = data + size;
	Assert(h->blocks.frontier < h->blocks.end);
	header->size = size;
	header->alignment = align;
	auto bytesBetweenEndOfHeaderAndStartOfData = data - ((u8 *)header + sizeof(HeapAllocationHeader));
	Assert(bytesBetweenEndOfHeaderAndStartOfData < U8_MAX);
	*(data - 1) = bytesBetweenEndOfHeaderAndStartOfData;
	return data;
}

void *ResizeHeapMemory(void *heap, void *m, s64 size)
{
	// @TODO
	FreeHeapMemory(heap, m);
	return AllocateHeapMemory(heap, size);
}

void FreeHeapMemory(void *heap, void *m)
{
	auto h = (HeapAllocator *)heap;
	auto bytesBetweenEndOfHeaderAndStartOfData = *((u8 *)m - 1);
	auto header = (HeapAllocationHeader *)((u8 *)m - bytesBetweenEndOfHeaderAndStartOfData);
	AppendToArray(&h->free, header);
}

void ClearHeapAllocator(void *heap)
{
	auto h = (HeapAllocator *)heap;
	ResetAllocatorBlocks(&h->blocks);
	ResizeArray(&h->free, 0);
}

void FreeHeapAllocator(void *heap)
{
	auto h = (HeapAllocator *)heap;
	// @TODO
	ResizeArray(&h->free, 0);
}
