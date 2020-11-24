#include "../CPU.h"

namespace cpu
{

void SpinWaitHint()
{
	_mm_pause();
}

s64 ProcessorCount()
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}

s64 PageSize()
{
	return sysconf(_SC_PAGESIZE);
}

}
