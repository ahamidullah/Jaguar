#include "Memory.h"
#include "ContextAllocator.h"

namespace Memory
{

void *Allocate(s64 size)
{
	return ContextAllocator()->Allocate(size);
}

void *AllocateAligned(s64 size, s64 align)
{
	return ContextAllocator()->AllocateAligned(size, align);
}

void *Resize(void *mem, s64 size)
{
	return ContextAllocator()->Resize(mem, size);
}

void Deallocate(void *mem)
{
	ContextAllocator()->Deallocate(mem);
}

PointerInt AlignAddress(PointerInt addr, s64 align)
{
	// Code from Game Engine Architecture (2018).
	Assert(align > 0);
	auto mask = align - 1;
	Assert((align & mask) == 0); // Power of 2.
	return (addr + mask) & ~mask;
}

void *AlignPointer(void *addr, s64 align)
{
	return (void *)AlignAddress((PointerInt)addr, align);
}

}
