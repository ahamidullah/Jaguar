#pragma once

#include "Basic/Array.h"

namespace Memory
{

struct BlockAllocator
{
	s64 blockSize;
	Allocator *allocator;
	Array<u8 *> used;
	Array<u8 *> unused;
	u8 *frontier;
	u8 *end;

	void AddBlock();
	void *Allocate(s64 size, s64 align);
	void *AllocateWithHeader(s64 size, s64 align);
	void Clear();
	void Free();
};

}
