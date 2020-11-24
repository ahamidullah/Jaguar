#pragma once

#include "Basic/Container/Array.h"

namespace Memory
{

struct BlockAllocator
{
	s64 blockSize;
	Allocator *allocator;
	array::Array<u8 *> used;
	array::Array<u8 *> unused;
	u8 *frontier;
	u8 *end;

	void AddBlock();
	void *Allocate(s64 size, s64 align);
	void *AllocateWithHeader(s64 size, s64 align);
	void Clear();
	void Free();
};

}
