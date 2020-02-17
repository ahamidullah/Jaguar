#pragma once

void *AllocatePlatformMemory(size_t size);
void FreePlatformMemory(void *memory, size_t size);
size_t GetPageSize();
