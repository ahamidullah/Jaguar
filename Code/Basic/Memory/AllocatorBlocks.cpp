#include "AllocatorBlocks.h"

namespace Memory
{

BlockAllocator NewBlockAllocator(s64 blockSize, s64 blockCount, Allocator *blockAlloc, Allocator *arrayAlloc)
{
	auto b = BlockAllocator
	{
		.blockSize = blockSize,
		.allocator = blockAlloc,
		.used = NewArrayWithCapacityIn<u8 *>(arrayAlloc, blockCount),
		.unused = NewArrayWithCapacityIn<u8 *>(arrayAlloc, blockCount),
	};
	for (auto i = 0; i < blockCount; i += 1)
	{
		b.unused.Append((u8 *)blockAlloc->Allocate(blockSize));
	}
	return b;
}

void BlockAllocator::AddBlock()
{
	Assert(this->blockSize > 0);
	if (this->unused.count > 0)
	{
		this->used.Append(this->unused.Pop());
	}
	else
	{
		this->used.Append((u8 *)this->allocator->Allocate(this->blockSize));
	}
	this->frontier = *this->used.Last();
	this->end = this->frontier + this->blockSize;
};

void *BlockAllocator::Allocate(s64 size, s64 align)
{
	Assert(size + (align - 1) <= this->blockSize);
	if (!this->frontier || (u8 *)AlignPointer(this->frontier, align) + size > this->end)
	{
		this->AddBlock();
	}
	this->frontier = (u8 *)AlignPointer(this->frontier, align);
	auto p = this->frontier;
	this->frontier += size;
	return p;
}

void *BlockAllocator::AllocateWithHeader(s64 size, s64 align)
{
	auto maxSize = size + sizeof(AllocationHeader) + (alignof(AllocationHeader) - 1) + align;
	Assert(maxSize <= this->blockSize);
	auto mem = this->Allocate(size + sizeof(AllocationHeader) + align, alignof(AllocationHeader));
	return SetAllocationHeaderAndData(mem, size, align);
}

void BlockAllocator::Clear()
{
	if (this->frontier)
	{
		this->unused.AppendAll(this->used);
		this->used.Resize(0);
		this->frontier = NULL;
	}
}

void BlockAllocator::Free()
{
	// @TODO
}

}

