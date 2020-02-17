#include <execinfo.h>

#include "Platform/Memory.h"

#define MAP_ANONYMOUS 0x20

void *PlatformAllocateMemory(size_t size)
{
	void *memory = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (memory == (void *)-1)
	{
		Assert(0); // @TODO
	}
	return memory;
}

void PlatformFreeMemory(void *memory, size_t size)
{
	if (munmap(memory, size) == -1)
	{
		LogPrint(ERROR_LOG, "failed to free platform memory: %s\n", PlatformGetError());
	}
}

size_t PlatformGetPageSize()
{
	return sysconf(_SC_PAGESIZE);
}

void PlatformPrintStacktrace()
{
	LogPrint(INFO_LOG, "Stack trace:\n");

	const u32 addressBufferSize = 100;
	void *addresses[addressBufferSize];
	s32 addressCount = backtrace(addresses, addressBufferSize);
	if (addressCount == addressBufferSize)
	{
		LogPrint(ERROR_LOG, "stack trace is probably truncated.\n");
	}

	char **strings = backtrace_symbols(addresses, addressCount);
	if (!strings)
	{
		LogPrint(ERROR_LOG, "failed to get function names\n");
		return;
	}
	for (s32 i = 0; i < addressCount; i++)
	{
		LogPrint(INFO_LOG, "\t%s\n", strings[i]);
	}
	free(strings);
}
