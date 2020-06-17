#include "../CPU.h"

s64 GetCPUProcessorCount()
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}

s64 GetCPUPageSize();
{
	static auto pageSize = sysconf(_SC_PAGESIZE);
	return pageSize;
}
