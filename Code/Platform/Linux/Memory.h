#pragma once

void *PlatformAllocateMemory(size_t size);
void PlatformFreeMemory(void *memory, size_t size);
size_t PlatformGetPageSize();
void PlatformPrintStacktrace();
