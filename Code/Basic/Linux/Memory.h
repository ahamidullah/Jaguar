#pragma once

void *AllocatePlatformMemory(s64 size);
void FreePlatformMemory(void *memory, s64 size);
size_t GetPageSize();
