#pragma once

#if __linux__
	#include "MemoryLinux.h"
#else
	#error Unsupported platform.
#endif

namespace Memory
{

void *Allocate(s64 size);
void *AllocateAligned(s64 size, s64 align);
void *Resize(void *mem, s64 newSize);
void Deallocate(void *mem);
PointerInt AlignAddress(PointerInt addr, s64 align);
void *AlignPointer(void *addr, s64 align);

}
