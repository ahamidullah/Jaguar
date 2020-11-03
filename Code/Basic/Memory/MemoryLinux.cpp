#include "Memory.h"
#include "../Log.h"
#include "../PCH.h"

namespace Memory
{

#define MAP_ANONYMOUS 0x20

void *PlatformAllocate(s64 size)
{
	auto mem = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (mem == (void *)-1)
	{
		Abort("Memory", "Failed to allocate memory: %k.", PlatformError());
	}
	return mem;
}

void PlatformDeallocate(void *mem, s64 size)
{
	if (munmap(mem, size) == -1)
	{
		Abort("Memory", "Failed to deallocate memory: %k.", PlatformError());
	}
}

}
