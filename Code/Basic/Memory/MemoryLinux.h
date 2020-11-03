#pragma once

#include "Common.h"

namespace Memory
{

void *PlatformAllocate(s64 size);
void PlatformDeallocate(void *mem, s64 size);

}
