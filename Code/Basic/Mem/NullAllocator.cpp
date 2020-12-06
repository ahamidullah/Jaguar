#include "NullAllocator.h"

namespace mem
{

// @TODO: Get rid of the NullAllocator...
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

}
