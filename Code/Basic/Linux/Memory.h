#pragma once

#include "Common.h"

void *AllocatePlatformMemory(s64 size);
void DeallocatePlatformMemory(void *mem, s64 size);
