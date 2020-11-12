#include "StackAllocator.h"

namespace Memory
{

void *StackAllocator::Allocate(s64 size)
{
	return this->AllocateAligned(size, DefaultAlignment);
}

void *StackAllocator::AllocateWithHeader(s64 size, s64 align)
{
	auto mem = (u8 *)AlignPointer(this->head, align);
	this->head = mem + size;
	if (this->head - this->buffer > this->size)
	{
		Abort("Memory", "Stack allocator ran out of space.");
	}
	return mem;
}

void *StackAllocator::AllocateAligned(s64 size, s64 align)
{
	return this->AllocateWithHeader(size, align);
}

void *StackAllocator::Resize(void *mem, s64 newSize)
{
	return this->Allocate(newSize);
}

void StackAllocator::Deallocate(void *mem)
{
}

void StackAllocator::Clear()
{
	this->head = this->buffer;
}

void StackAllocator::Free()
{
}

StackAllocator NewStackAllocator(array::View<u8> mem)
{
	auto a = StackAllocator{};
	a.buffer = mem.elements;
	a.head = mem.elements;
	a.size = mem.count;
	return a;
}

}
