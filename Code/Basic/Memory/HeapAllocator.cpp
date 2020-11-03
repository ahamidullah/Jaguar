#include "HeapAllocator.h"

namespace Memory
{

HeapAllocator NewHeapAllocator(s64 blockSize, s64 blockCount, Allocator *blockAlloc, Allocator *arrayAlloc)
{
	auto a = HeapAllocator{};
	a.blocks = NewBlockAllocator(blockSize, blockCount, blockAlloc, arrayAlloc);
	a.free = NewArrayIn<AllocationHeader *>(arrayAlloc, 0);
	return a;
}

void *HeapAllocator::Allocate(s64 size)
{
	return this->AllocateAligned(size, DefaultAlignment);
}

void *HeapAllocator::AllocateAligned(s64 size, s64 align)
{
	return this->blocks.AllocateWithHeader(size, align);
}

void *HeapAllocator::Resize(void *mem, s64 newSize)
{
	auto h = GetAllocationHeader(mem);
	Assert(newSize >= h->size);
	auto newMem = this->blocks.AllocateWithHeader(newSize, h->alignment);
	CopyArray(NewArrayView((u8 *)mem, h->size), NewArrayView((u8 *)newMem, h->size));
	this->free.Append(h);
	return newMem;
}

void HeapAllocator::Deallocate(void *mem)
{
	auto h = GetAllocationHeader(mem);
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

}
