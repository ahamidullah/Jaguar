#pragma once

#include "Basic/Container/Array.h"

namespace mem
{

struct BlockAllocator
{
	s64 blockSize;
	Allocator *allocator;
	arr::array<u8 *> used;
	arr::array<u8 *> unused;
	u8 *frontier;
	u8 *end;

	void AddBlock();
	void *Allocate(s64 size, s64 align);
	void *AllocateWithHeader(s64 size, s64 align);
	void Clear();
	void Free();
};

}
