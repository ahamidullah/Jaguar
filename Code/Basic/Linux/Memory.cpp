#include "../Memory.h"
#include "../Log.h"
#include "../Process.h"

#define MAP_ANONYMOUS 0x20

void *AllocatePlatformMemory(s64 size)
{
	auto mem = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (mem == (void *)-1)
	{
		Abort("Failed to allocate platform memory: %k.", GetPlatformError());
	}
	return mem;
}

void FreePlatformMemory(void *mem, s64 size)
{
	if (munmap(mem, size) == -1)
	{
		LogPrint(LogLevelError, "Memory", "Failed to free platform memory: %k.\n", GetPlatformError());
	}
}
