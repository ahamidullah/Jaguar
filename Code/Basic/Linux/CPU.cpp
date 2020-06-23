#include "../CPU.h"

s64 CPUProcessorCount()
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}

s64 CPUPageSize()
{
	static auto pageSize = sysconf(_SC_PAGESIZE);
	return pageSize;
}
