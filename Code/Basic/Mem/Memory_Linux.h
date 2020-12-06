#pragma once

#include "Common.h"

namespace mem
{

void *PlatformAllocate(s64 size);
void PlatformDeallocate(void *mem, s64 size);

}
