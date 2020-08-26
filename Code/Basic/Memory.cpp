#include "Memory.h"
#include "CPU.h"
#include "Log.h"
#include "Process.h"
#include "Spinlock.h"
#include "Fiber.h"

// TODO: Change free to deallocate.

const auto DefaultMemoryAlignment = 16;
const auto GlobalHeapBlockSize = MegabytesToBytes(64);
const auto GlobalHeapInitialBlockCount = 32;

struct GlobalHeapBlockAllocator : Allocator
{
	void *Allocate(s64 size);
	void *AllocateAligned(s64 size, s64 align);
	void *Resize(void *mem, s64 size);
	void Deallocate(void *mem);
	void Clear();
	void Free();
};

void *GlobalHeapBlockAllocator::Allocate(s64 size)
{
	Assert(size == GlobalHeapBlockSize);
	Assert(size % CPUPageSize() == 0);
	return AllocatePlatformMemory(size);
}

void *GlobalHeapBlockAllocator::AllocateAligned(s64 size, s64 align)
{
	Abort("Memory", "Unsupported call to AllocateAligned in GlobalHeapBlockAllocator.");
	return NULL;
}

void *GlobalHeapBlockAllocator::Resize(void *mem, s64 size)
{
	Abort("Memory", "Unsupported call to Resize in GlobalHeapBlockAllocator.");
	return NULL;
}

void GlobalHeapBlockAllocator::Deallocate(void *mem)
{
	DeallocatePlatformMemory(mem, GlobalHeapBlockSize);
}

void GlobalHeapBlockAllocator::Clear()
{
	Abort("Memory", "Unsupported call to Clear in GlobalHeapBlockAllocator.");
}

void GlobalHeapBlockAllocator::Free()
{
	Abort("Memory", "Unsupported call to Free in GlobalHeapBlockAllocator.");
}

struct GlobalHeapArrayAllocator : Allocator
{
	void *Allocate(s64 size);
	void *AllocateAligned(s64 size, s64 align);
	void *Resize(void *mem, s64 size);
	void Deallocate(void *mem);
	void Clear();
	void Free();
};

void *GlobalHeapArrayAllocator::Allocate(s64 size)
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

void *GlobalHeapArrayAllocator::AllocateAligned(s64 size, s64 align)
{
	Abort("Memory", "Unsupported call to AllocateAligned in GlobalHeapArrayAllocator.");
	return NULL;
}

void *GlobalHeapArrayAllocator::Resize(void *mem, s64 size)
{
	this->Deallocate(mem);
	return this->Allocate(size);
}

void GlobalHeapArrayAllocator::Deallocate(void *mem)
{
	auto size = *(s64 *)((u8 *)mem - sizeof(s64));
	DeallocatePlatformMemory(mem, size);
}

void GlobalHeapArrayAllocator::Clear()
{
	Abort("Memory", "Unsupported call to Clear in GlobalHeapArrayAllocator.");
}

void GlobalHeapArrayAllocator::Free()
{
	Abort("Memory", "Unsupported call to Free in GlobalHeapArrayAllocator.");
}

void *GlobalHeapAllocator::Allocate(s64 size)
{
	this->lock.Lock();
	Defer(this->lock.Unlock());
	return this->heap.Allocate(size);
}

GlobalHeapAllocator NewGlobalHeapAllocator(HeapAllocator h)
{
	// This constructor function exists because C++ sucks and does not let you use brace initialization if the struct has a virtual method.
	// Why? I don't know! So, we'll just use this awkward function instead.
	auto a = GlobalHeapAllocator{};
	a.heap = h;
	return a;
}

void *GlobalHeapAllocator::AllocateAligned(s64 size, s64 align)
{
	this->lock.Lock();
	Defer(this->lock.Unlock());
	return this->heap.AllocateAligned(size, align);
}

void *GlobalHeapAllocator::Resize(void *mem, s64 size)
{
	this->lock.Lock();
	Defer(this->lock.Unlock());
	return this->heap.Resize(mem, size);
}

void GlobalHeapAllocator::Deallocate(void *mem)
{
	this->lock.Lock();
	Defer(this->lock.Unlock());
	this->heap.Deallocate(mem);
}

void GlobalHeapAllocator::Clear()
{
	this->lock.Lock();
	Defer(this->lock.Unlock());
	this->heap.Clear();
}

void GlobalHeapAllocator::Free()
{
	this->lock.Lock();
	Defer(this->lock.Unlock());
	this->heap.Free();
}

static auto globalHeapBlockAllocator = GlobalHeapBlockAllocator{};
static auto globalHeapArrayAllocator = GlobalHeapArrayAllocator{};
static auto globalHeapAllocator = NewGlobalHeapAllocator(NewHeapAllocator(GlobalHeapBlockSize, GlobalHeapInitialBlockCount, &globalHeapBlockAllocator, &globalHeapArrayAllocator));

GlobalHeapAllocator *GlobalHeap()
{
	return &globalHeapAllocator;
}

auto baseContextAllocator = (Allocator *)&globalHeapAllocator;
ThreadLocal auto contextAllocatorStack = Array<Allocator *>{};
ThreadLocal auto contextAllocator = (Allocator *)&globalHeapAllocator;

void PushContextAllocator(Allocator *a)
{
	if (RunningFiber())
	{
		RunningFiber()->contextAllocatorStack.Append(a);
		RunningFiber()->contextAllocator = a;
	}
	else
	{
		contextAllocatorStack.Append(a);
		contextAllocator = a;
	}
}

void PopContextAllocator()
{
	auto stk = (Array<Allocator *> *){};
	auto ctx = (Allocator **){};
	auto base = (Allocator *){};
	if (RunningFiber())
	{
		base = RunningFiber()->baseContextAllocator;
		stk = &RunningFiber()->contextAllocatorStack;
		ctx = &RunningFiber()->contextAllocator;
	}
	else
	{
		base = baseContextAllocator;
		stk = &contextAllocatorStack;
		ctx = &contextAllocator;
	}
	if (stk->count == 0)
	{
		LogPrint(ErrorLog, "Memory", "Tried to pop empty context allocator stack.\n");
		return;
	}
	stk->Resize(stk->count - 1);
	if (stk->count == 0)
	{
		*ctx = base;
	}
	else
	{
		*ctx = (*stk)[stk->count - 1];
	}
}

Allocator *ContextAllocator()
{
	if (RunningFiber())
	{
		return RunningFiber()->contextAllocator;
	}
	return contextAllocator;
}

void *AllocateMemory(s64 size)
{
	return ContextAllocator()->Allocate(size);
}

void *AllocateAlignedMemory(s64 size, s64 align)
{
	return ContextAllocator()->AllocateAligned(size, align);
}

void *ResizeMemory(void *mem, s64 size)
{
	return ContextAllocator()->Resize(mem, size);
}

void DeallocateMemory(void *mem)
{
	ContextAllocator()->Deallocate(mem);
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

#if 0
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
#endif

AllocatorBlocks NewAllocatorBlocks(s64 blockSize, s64 blockCount, Allocator *blockAlloc, Allocator *arrayAlloc)
{
	auto b = AllocatorBlocks
	{
		.blockSize = blockSize,
		.allocator = blockAlloc,
		.used = NewArrayWithCapacityIn<u8 *>(arrayAlloc, 0, blockCount),
		.unused = NewArrayWithCapacityIn<u8 *>(arrayAlloc, 0, blockCount),
	};
	for (auto i = 0; i < blockCount; i++)
	{
		b.unused.Append((u8 *)blockAlloc->Allocate(blockSize));
	}
	return b;
}

void AllocatorBlocks::NewBlock()
{
	Assert(this->blockSize > 0);
	if (this->unused.count > 0)
	{
		this->used.Append(this->unused[this->unused.count - 1]);
		this->unused.Resize(this->unused.count - 1);
	}
	else
	{
		this->used.Append((u8 *)this->allocator->Allocate(this->blockSize));
	}
	this->frontier = this->used[this->used.count - 1];
	this->end = this->frontier + this->blockSize;
};

void *AllocatorBlocks::Allocate(s64 size, s64 align)
{
	Assert(size + (align - 1) <= this->blockSize);
	if (!this->frontier || (u8 *)AlignPointer(this->frontier, align) + size > this->end)
	{
		this->NewBlock();
	}
	this->frontier = (u8 *)AlignPointer(this->frontier, align);
	auto p = this->frontier;
	this->frontier += size;
	return p;
}

void AllocatorBlocks::Clear()
{
	if (this->frontier)
	{
		this->unused.AppendAll(this->used);
		this->used.Resize(0);
		this->frontier = NULL;
	}
}

void AllocatorBlocks::Free()
{
	// @TODO
}

PoolAllocator NewPoolAllocator(s64 blockSize, s64 blockCount, Allocator *blockAlloc, Allocator *arrayAlloc)
{
	auto a = PoolAllocator{};
	a.blocks = NewAllocatorBlocks(blockSize, blockCount, blockAlloc, arrayAlloc);
	return a;
}

void *PoolAllocator::Allocate(s64 size)
{
	return this->AllocateAligned(size, DefaultMemoryAlignment);
}

void *PoolAllocator::AllocateAligned(s64 size, s64 align)
{
	Assert(align > 0);
	return this->blocks.Allocate(size, align);
}

void *PoolAllocator::Resize(void *mem, s64 size)
{
	return this->Allocate(size);
}

void PoolAllocator::Deallocate(void *mem)
{
}

void PoolAllocator::Clear()
{
	this->blocks.Clear();
}

void PoolAllocator::Free()
{
	// @TODO
}

SlotAllocator NewSlotAllocator(s64 slotSize, s64 slotAlign, s64 slotCount, s64 slotsPerBlock, Allocator *blockAlloc, Allocator *arrayAlloc)
{
	auto blockCount = (slotCount + (slotsPerBlock - 1)) / slotsPerBlock; // Divide and round up.
	auto a = SlotAllocator{};
	a.blocks = NewAllocatorBlocks(slotsPerBlock * slotSize, blockCount, blockAlloc, arrayAlloc);
	a.slotSize = slotSize;
	a.slotAlignment = slotAlign;
	a.freeSlots = NewArrayIn<void *>(arrayAlloc, 0);
	return a;
}

void *SlotAllocator::Allocate(s64 size)
{
	Assert(size == this->slotSize);
	if (this->freeSlots.count > 0)
	{
		auto result = this->freeSlots[this->freeSlots.count - 1];
		this->freeSlots.Resize(this->freeSlots.count - 1);
		return result;
	}
	return this->blocks.Allocate(size, this->slotAlignment);
}

void *SlotAllocator::AllocateAligned(s64 size, s64 align)
{
	// Allocating aligned memory from a slot allocator does not make sense.
	// Just return a regular allocation.
	return this->Allocate(size);
}

void *SlotAllocator::Resize(void *mem, s64 size)
{
	Abort("Memory", "Attempted to resize a slot memory allocation.");
	return NULL;
}

void SlotAllocator::Deallocate(void *mem)
{
	this->freeSlots.Append(mem);
}

void SlotAllocator::Clear()
{
	this->blocks.Clear();
	this->freeSlots.Resize(0);
}

void SlotAllocator::Free()
{
	// @TODO
	this->freeSlots.Resize(0);
}

HeapAllocator NewHeapAllocator(s64 blockSize, s64 blockCount, Allocator *blockAlloc, Allocator *arrayAlloc)
{
	auto a = HeapAllocator{};
	a.blocks = NewAllocatorBlocks(blockSize, blockCount, blockAlloc, arrayAlloc);
	a.free = NewArrayIn<HeapAllocationHeader *>(arrayAlloc, 0);
	return a;
}

void *HeapAllocator::Allocate(s64 size)
{
	return this->AllocateAligned(size, DefaultMemoryAlignment);
}

void *HeapAllocator::AllocateAligned(s64 size, s64 align)
{
	auto maxSize = size + sizeof(HeapAllocationHeader) + (alignof(HeapAllocationHeader) - 1) + align;
	Assert(maxSize <= this->blocks.blockSize);
	auto header = (HeapAllocationHeader *)this->blocks.Allocate(size + sizeof(HeapAllocationHeader) + align, alignof(HeapAllocationHeader));
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
	header->size = size;
	header->alignment = align;
	auto bytesBetweenEndOfHeaderAndStartOfData = data - ((u8 *)header + sizeof(HeapAllocationHeader));
	Assert(bytesBetweenEndOfHeaderAndStartOfData < U8Max);
	*(data - 1) = bytesBetweenEndOfHeaderAndStartOfData;
	return data;
}

void *HeapAllocator::Resize(void *mem, s64 size)
{
	// @TODO
	this->Deallocate(mem);
	return this->Allocate(size);
}

void HeapAllocator::Deallocate(void *mem)
{
	auto bytesBetweenEndOfHeaderAndStartOfData = *((u8 *)mem - 1);
	auto header = (HeapAllocationHeader *)((u8 *)mem - bytesBetweenEndOfHeaderAndStartOfData);
	this->free.Append(header);
}

void HeapAllocator::Clear()
{
	this->blocks.Clear();
	this->free.Resize(0);
}

void HeapAllocator::Free()
{
	// @TODO
	this->free.Resize(0);
}
