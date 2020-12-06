#include "GlobalHeap.h"
#include "Memory.h"

namespace mem
{

const auto GlobalHeapBlockSize = 64 * Megabyte;

struct GlobalHeapBlockAllocator : allocator
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
	return PlatformAllocate(size);
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
	PlatformDeallocate(mem, GlobalHeapBlockSize);
}

void GlobalHeapBlockAllocator::Clear()
{
	Abort("Memory", "Unsupported call to Clear in GlobalHeapBlockAllocator.");
}

void GlobalHeapBlockAllocator::Free()
{
	Abort("Memory", "Unsupported call to Free in GlobalHeapBlockAllocator.");
}

struct GlobalHeapArrayAllocator : allocator
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
	// In the interest of keeping this code simple, we're gonna assume that no alignment of the data is necessary.  This should be fine
	// since this allocator is only ever used to allocate arrays of pointers for the global heap allocator.
	Assert(alignof(u8 *) % alignof(s64) == 0);
	size = AlignAddress(size + sizeof(size), CPUPageSize());
	auto alloc = PlatformAllocate(size);
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
	PlatformDeallocate(mem, size);
	return newMem;
}

void GlobalHeapArrayAllocator::Deallocate(void *mem)
{
	mem = (u8 *)mem - sizeof(s64);
	auto size = *(s64 *)mem;
	PlatformDeallocate(mem, size);
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
	if (this->lock.IsLocked() && this->lockThreadID == ThreadID())
	{
		return this->backup.Allocate(size);
	}
	this->lock.Lock();
	this->lockThreadID = ThreadID();
	Defer(
	{
		this->lockThreadID = -1;
		this->lock.Unlock();
	});
	return this->heap.Allocate(size);
}

void *GlobalHeapAllocator::AllocateAligned(s64 size, s64 align)
{
	if (this->lock.IsLocked() && this->lockThreadID == ThreadID())
	{
		return this->backup.AllocateAligned(size, align);
	}
	this->lock.Lock();
	this->lockThreadID = ThreadID();
	Defer(
	{
		this->lockThreadID = -1;
		this->lock.Unlock();
	});
	return this->heap.AllocateAligned(size, align);
}

void *GlobalHeapAllocator::Resize(void *mem, s64 newSize)
{
	if (this->lock.IsLocked() && this->lockThreadID == ThreadID())
	{
		return this->backup.Resize(mem, newSize);
	}
	this->lock.Lock();
	this->lockThreadID = ThreadID();
	Defer(
	{
		this->lockThreadID = -1;
		this->lock.Unlock();
	});
	return this->heap.Resize(mem, newSize);
}

void GlobalHeapAllocator::Deallocate(void *mem)
{
	if (this->lock.IsLocked() && this->lockThreadID == ThreadID())
	{
		this->backup.Deallocate(mem);
	}
	this->lock.Lock();
	this->lockThreadID = ThreadID();
	Defer(
	{
		this->lockThreadID = -1;
		this->lock.Unlock();
	});
	this->heap.Deallocate(mem);
}

void GlobalHeapAllocator::Clear()
{
	if (this->lock.IsLocked() && this->lockThreadID == ThreadID())
	{
		return;
	}
	this->lock.Lock();
	this->lockThreadID = ThreadID();
	Defer(
	{
		this->lockThreadID = -1;
		this->lock.Unlock();
	});
	this->heap.Clear();
}

void GlobalHeapAllocator::Free()
{
	if (this->lock.IsLocked() && this->lockThreadID == ThreadID())
	{
		return;
	}
	this->lock.Lock();
	this->lockThreadID = ThreadID();
	Defer(
	{
		this->lockThreadID = -1;
		this->lock.Unlock();
	});
	this->heap.Free();
}

// @TODO: Make the backup allocator its own thing, not a stack allocator.
// Have algorithms for freeing the stack allocator's memory. Using free regions, etc.
// Could reuse algorithms from heap allocator. It's basically a single block heap allocator.
GlobalHeapAllocator *GlobalHeap()
{
	static auto blockAlloc = GlobalHeapBlockAllocator{};
	static auto arrayAlloc = GlobalHeapArrayAllocator{};
	static auto alloc = GlobalHeapAllocator{};
	static auto init = false;
	if (!init)
	{
		alloc.lock.Lock();
		alloc.lockThreadID = ThreadID();
		Defer(
		{
			alloc.lockThreadID = -1;
			alloc.lock.Unlock();
		});
		if (!init)
		{
			alloc.backup = NewStackAllocator(alloc.backupBuffer);
			alloc.heap = NewHeapAllocator(GlobalHeapBlockSize, 32, &blockAlloc, &arrayAlloc);
			init = true;
		}
	}
	return &alloc;
}

}
