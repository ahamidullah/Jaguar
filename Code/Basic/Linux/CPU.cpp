#include "../CPU.h"

s64 CPUProcessorCount()
{
	static auto pc = sysconf(_SC_NPROCESSORS_ONLN);
	return pc;
}

s64 CPUPageSize()
{
	static auto ps = sysconf(_SC_PAGESIZE);
	return ps;
}
