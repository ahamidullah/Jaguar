#include "Memory.h"
#include "CPU.h"
#include "Log.h"
#include "Process.h"
#include "Spinlock.h"
#include "Fiber.h"

StackAllocator *BackupGlobalAllocator()
{
	static auto mem = StaticArray<u8, KilobytesToBytes(32)>{};
	static auto a = NewStackAllocator(mem);
	return &a;
}

Allocator **ContextAllocatorPointer()
{
	static ThreadLocal auto contextAllocator = (Allocator *){};
	if (!contextAllocator)
	{
		contextAllocator = BackupGlobalAllocator();
	}
	return &contextAllocator;
}

Allocator *ContextAllocator()
{
	if (RunningFiber())
	{
		return RunningFiber()->contextAllocator;
	}
	return *ContextAllocatorPointer();
}

void SetContextAllocator(Allocator *a)
{
	if (RunningFiber())
	{
		RunningFiber()->contextAllocator = a;
		return;
	}
	*ContextAllocatorPointer() = a;
}

Array<Allocator *> *ContextAllocatorStack()
{
	static ThreadLocal auto contextAllocatorStack = Array<Allocator *>{};
	return &contextAllocatorStack;
}

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

void *GlobalHeapBlockAllocator::Resize(void *mem, s64 newSize)
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
	void *Resize(void *mem, s64 newSize);
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

void *GlobalHeapArrayAllocator::Resize(void *mem, s64 newSize)
{
	mem = (u8 *)mem - sizeof(s64);
	auto size = *(s64 *)mem;
	auto newMem = this->Allocate(newSize);
	DeallocatePlatformMemory(mem, size);
	return newMem;
}

void GlobalHeapArrayAllocator::Deallocate(void *mem)
{
	mem = (u8 *)mem - sizeof(s64);
	auto size = *(s64 *)mem;
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

GlobalHeapAllocator NewGlobalHeapAllocator(HeapAllocator h)
{
	// This constructor function exists because C++ sucks and does not let you use brace initialization if the struct has a virtual method.
	// Why? I don't know! So, we'll just use this awkward function instead.
	auto a = GlobalHeapAllocator{};
	a.heap = h;
	return a;
}

void *GlobalHeapAllocator::Allocate(s64 size)
{
	this->lock.Lock();
	Defer(this->lock.Unlock());
	return this->heap.Allocate(size);
}

void *GlobalHeapAllocator::AllocateAligned(s64 size, s64 align)
{
	this->lock.Lock();
	Defer(this->lock.Unlock());
	return this->heap.AllocateAligned(size, align);
}

void *GlobalHeapAllocator::Resize(void *mem, s64 newSize)
{
	this->lock.Lock();
	Defer(this->lock.Unlock());
	return this->heap.Resize(mem, newSize);
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

bool *GlobalHeapInitPointer()
{
	static auto init = false;
	return &init;
}

bool IsGlobalHeapInitialized()
{
	return *GlobalHeapInitPointer();
}

Allocator *GlobalHeap()
{
	static auto blk = GlobalHeapBlockAllocator{};
	static auto arr = GlobalHeapArrayAllocator{};
	static auto alloc = GlobalHeapAllocator{};
	static auto initializing = false;
	if (!initializing && !IsGlobalHeapInitialized())
	{
		// Try to initialize the global heap.
		Assert(ThreadCount() == 1);
		initializing = true;
		alloc.heap = NewHeapAllocator(GlobalHeapBlockSize, GlobalHeapInitialBlockCount, &blk, &arr);
		Abort("Memory", "WHOOPS.");
		*GlobalHeapInitPointer() = true;
		Assert(ContextAllocatorStack()->count == 0);
		// We no longer need to use the backup global allocator!
		SetContextAllocator(&alloc);
	}
	else if (initializing && !IsGlobalHeapInitialized())
	{
		// Well, we tried to initialize the global heap, but failed, somehow.
		// Fallback to the backup, fixed-size buffer allocator.
		Assert(ThreadCount() == 1);
		return BackupGlobalAllocator();
	}
	return &alloc;
}

void InitializeMemory()
{
	GlobalHeap(); // Ensure that the global heap is initialized...
}

void PushContextAllocator(Allocator *a)
{
	if (!IsGlobalHeapInitialized())
	{
		SetContextAllocator(a); // @TODO: What should we do here. Small, fixed size backup context allocator stack?
		return;
	}
	if (RunningFiber())
	{
		RunningFiber()->contextAllocatorStack.Append(a);
		RunningFiber()->contextAllocator = a;
		return;
	}
	ContextAllocatorStack()->Append(a);
	SetContextAllocator(a);
}

void PopContextAllocator()
{
	if (!IsGlobalHeapInitialized())
	{
		return; // @TODO
	}
	auto stk = (Array<Allocator *> *){};
	auto ctx = (Allocator **){};
	if (RunningFiber())
	{
		stk = &RunningFiber()->contextAllocatorStack;
		ctx = &RunningFiber()->contextAllocator;
	}
	else
	{
		stk = ContextAllocatorStack();
		ctx = ContextAllocatorPointer();
	}
	if (stk->count == 0)
	{
		LogError("Memory", "Tried to pop empty context allocator stack.\n");
		return;
	}
	stk->Resize(stk->count - 1);
	if (stk->count == 0)
	{
		*ctx = GlobalHeap();
	}
	else
	{
		*ctx = (*stk)[stk->count - 1];
	}
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

AllocationHeader *GetMemoryHeader(void *mem)
{
	auto bytesFromDataToHeader = *((u8 *)mem - 1);
	return (AllocationHeader *)((u8 *)mem - bytesFromDataToHeader);
}

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

void *AllocatorBlocks::AllocateWithHeader(s64 size, s64 align)
{
	auto maxSize = size + sizeof(AllocationHeader) + (alignof(AllocationHeader) - 1) + align;
	Assert(maxSize <= this->blockSize);
	auto header = (AllocationHeader *)this->Allocate(size + sizeof(AllocationHeader) + align, alignof(AllocationHeader));
	auto data = (u8 *)header + sizeof(AllocationHeader);
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
	auto bytesFromDataToHeader = data - (u8 *)header;
	Assert(bytesFromDataToHeader < U8Max);
	*(data - 1) = bytesFromDataToHeader;
	return data;
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
	return this->blocks.AllocateWithHeader(size, align);
}

void *PoolAllocator::Resize(void *mem, s64 newSize)
{
	auto h = GetMemoryHeader(mem);
	auto newMem = this->blocks.AllocateWithHeader(newSize, h->alignment);
	CopyArray(NewArrayView((u8 *)mem, h->size), NewArrayView((u8 *)newMem, h->size));
	return newMem;
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

void *SlotAllocator::Resize(void *mem, s64 newSize)
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
	a.free = NewArrayIn<AllocationHeader *>(arrayAlloc, 0);
	return a;
}

void *HeapAllocator::Allocate(s64 size)
{
	return this->AllocateAligned(size, DefaultMemoryAlignment);
}

void *HeapAllocator::AllocateAligned(s64 size, s64 align)
{
	return this->blocks.AllocateWithHeader(size, align);
}

void *HeapAllocator::Resize(void *mem, s64 newSize)
{
	auto h = GetMemoryHeader(mem);
	auto newMem = this->blocks.AllocateWithHeader(newSize, h->alignment);
	CopyArray(NewArrayView((u8 *)mem, h->size), NewArrayView((u8 *)newMem, h->size));
	this->free.Append(h);
	return newMem;
}

void HeapAllocator::Deallocate(void *mem)
{
	auto h = GetMemoryHeader(mem);
	this->free.Append(h);
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

void *NullAllocator::Allocate(s64 size)
{
	return NULL;
}

void *NullAllocator::AllocateAligned(s64 size, s64 align)
{
	return NULL;
}

void *NullAllocator::Resize(void *mem, s64 newSize)
{
	return NULL;
}

void NullAllocator::Deallocate(void *mem)
{
}

void NullAllocator::Clear()
{
}

void NullAllocator::Free()
{
}

struct NullAllocator *NullAllocator()
{
	static auto a = (struct NullAllocator){};
	return &a;
}

void *StackAllocator::Allocate(s64 size)
{
	return this->AllocateAligned(size, DefaultMemoryAlignment);
}

void *StackAllocator::AllocateWithHeader(s64 size, s64 align)
{
	this->head = (u8 *)AlignPointer(this->head, alignof(AllocationHeader));
	auto h = (AllocationHeader *)this->head;
	h->size = size;
	h->alignment = align;
	this->head += sizeof(AllocationHeader);
	auto oldHead = this->head;
	this->head = (u8 *)AlignPointer(this->head, align);
	if (this->head == oldHead)
	{
		this->head += align;
	}
	*(this->head - 1) = (IntegerPointer)(this->head - (u8 *)h);
	auto r = this->head;
	this->head += size;
	if (this->head - this->buffer > this->size)
	{
		ConsoleWrite("Ran out of stack allocator space...\n");
		SignalDebugBreakpoint();
		ExitProcess(ProcessFail);
	}
	return r;
}

void *StackAllocator::AllocateAligned(s64 size, s64 align)
{
	return this->AllocateWithHeader(size, align);
}

void *StackAllocator::Resize(void *mem, s64 newSize)
{
	auto h = GetMemoryHeader(mem);
	if (((u8 *)mem) + h->size == this->head)
	{
		h->size = newSize;
		this->head = ((u8 *)mem) + newSize;
		return mem;
	}
	auto newMem = this->AllocateWithHeader(newSize, h->alignment);
	CopyArray(NewArrayView((u8 *)mem, h->size), NewArrayView((u8 *)newMem, h->size));
	return newMem;
}

void StackAllocator::Deallocate(void *mem)
{
	auto h = GetMemoryHeader(mem);
	if (((u8 *)mem) + h->size == this->head)
	{
		this->head = (u8 *)h;
	}
}

void StackAllocator::Clear()
{
	this->head = this->buffer;
}

void StackAllocator::Free()
{
}

StackAllocator NewStackAllocator(ArrayView<u8> mem)
{
	auto a = StackAllocator{};
	a.buffer = mem.elements;
	a.head = mem.elements;
	a.size = mem.count;
	return a;
}
