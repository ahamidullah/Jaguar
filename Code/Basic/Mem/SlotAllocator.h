#pragma once

#include "Allocator.h"
#include "AllocatorBlocks.h"

namespace mem
{

struct SlotAllocator : Allocator
{
	BlockAllocator blocks;
	s64 slotSize;
	s64 slotAlignment;
	arr::array<void *> freeSlots;

	void *Allocate(s64 size);
	void *AllocateAligned(s64 size, s64 align);
	void *Resize(void *mem, s64 newSize);
	void Deallocate(void *mem);
	void Clear();
	void Free();
};

SlotAllocator NewSlotAllocator(s64 slotSize, s64 slotAlignment, s64 slotCount, s64 slotsPerBlock, Allocator *blockAlloc, Allocator *arrayAlloc);

}
