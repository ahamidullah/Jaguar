#pragma once

#include "../PCH.h"

#include "Code/Common.h"

void *AllocatePlatformMemory(s64 size);
void FreePlatformMemory(void *memory, s64 size);
