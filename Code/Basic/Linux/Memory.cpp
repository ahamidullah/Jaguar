#include "../PCH.h"
#include "../Memory.h"
#include "../Log.h"
#include "../Process.h"

#define MAP_ANONYMOUS 0x20

void *AllocatePlatformMemory(s64 size)
{
	auto mem = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (mem == (void *)-1)
	{
		Abort("Memory", "Failed to allocate platform memory: %k.", PlatformError());
	}
	return mem;
}

void DeallocatePlatformMemory(void *mem, s64 size)
{
	if (munmap(mem, size) == -1)
	{
		LogPrint(ErrorLog, "Memory", "Failed to deallocate platform memory: %k.\n", PlatformError());
	}
}

Array<String> Stacktrace()
{
	const auto maxAddrs = 100;
	auto addrs = StaticArray<void *, maxAddrs>{};
	auto numAddrs = backtrace(&addrs[0], maxAddrs);
	if (numAddrs == maxAddrs)
	{
		LogPrint(ErrorLog, "Memory", "Stack trace is probably truncated.\n");
	}
	auto trace = backtrace_symbols(&addrs[0], numAddrs);
	if (!trace)
	{
		LogPrint(ErrorLog, "Memory", "Failed to get stack trace function names.\n");
		return {};
	}
	auto st = NewArrayWithCapacity<String>(0, numAddrs);
	for (auto i = 0; i < numAddrs; i++)
	{
		st.Append(trace[i]);
	}
	free(trace);
	return st;
}
