#include "PoolAllocator.h"

namespace Memory
{

PoolAllocator NewPoolAllocator(s64 blockSize, s64 blockCount, Allocator *blockAlloc, Allocator *arrayAlloc)
{
	auto a = PoolAllocator{};
	a.blocks = NewBlockAllocator(blockSize, blockCount, blockAlloc, arrayAlloc);
	return a;
}

void *PoolAllocator::Allocate(s64 size)
{
	return this->AllocateAligned(size, DefaultAlignment);
}

void *PoolAllocator::AllocateAligned(s64 size, s64 align)
{
	Assert(align > 0);
	return this->blocks.AllocateWithHeader(size, align);
}

void *PoolAllocator::Resize(void *mem, s64 newSize)
{
	auto h = GetAllocationHeader(mem);
	auto newMem = this->blocks.AllocateWithHeader(newSize, h->alignment);
	array::Copy(array::NewView((u8 *)mem, h->size), array::NewView((u8 *)newMem, h->size));
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

}
