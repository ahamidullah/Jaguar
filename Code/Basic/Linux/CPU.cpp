#include "../CPU.h"

s64 CPUProcessorCount()
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}

s64 CPUPageSize()
{
	return sysconf(_SC_PAGESIZE);
}
