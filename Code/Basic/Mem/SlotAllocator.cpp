#include "SlotAllocator.h"

namespace mem
{

SlotAllocator NewSlotAllocator(s64 slotSize, s64 slotAlign, s64 slotCount, s64 slotsPerBlock, Allocator *blockAlloc, Allocator *arrayAlloc)
{
	auto nBlks = (slotCount + (slotsPerBlock - 1)) / slotsPerBlock; // Divide and round up.
	auto a = SlotAllocator{};
	a.blocks = NewBlockAllocator(slotsPerBlock * slotSize, nBlks, blockAlloc, arrayAlloc);
	a.slotSize = slotSize;
	a.slotAlignment = slotAlign;
	a.freeSlots.SetAllocator(arrayAlloc);
	return a;
}

void *SlotAllocator::Allocate(s64 size)
{
	Assert(size == this->slotSize);
	if (this->freeSlots.count > 0)
	{
		return this->freeSlots.Pop();
	}
	return this->blocks.Allocate(size, this->slotAlignment);
}

void *SlotAllocator::AllocateAligned(s64 size, s64 align)
{
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

}
