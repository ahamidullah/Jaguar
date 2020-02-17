#include <execinfo.h>
#include <sys/mman.h>
#include <unistd.h>

#define MAP_ANONYMOUS 0x20

void *AllocatePlatformMemory(size_t size)
{
	void *memory = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (memory == (void *)-1)
	{
		Assert(0); // @TODO
	}
	return memory;
}

void FreePlatformMemory(void *memory, size_t size)
{
	if (munmap(memory, size) == -1)
	{
		LogPrint(LogType::ERROR, "failed to free platform memory: %s\n", GetPlatformError());
	}
}

size_t GetPageSize()
{
	return sysconf(_SC_PAGESIZE);
}
