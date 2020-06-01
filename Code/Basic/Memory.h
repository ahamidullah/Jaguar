#pragma once

#if defined(__linux__)
	#include "Linux/Memory.h"
#else
	#error Unsupported platform.
#endif
#include "PCH.h"

#define AllocateStructMemory(type) AllocateStructMemoryActual<type>()
#define AllocateArrayMemory(type, count) AllocateArrayMemoryActual<type>(count)

template <typename T>
T *AllocateStructMemoryActual()
{
	auto p = malloc(sizeof(T)); // @TODO
	return new(p) T;
}

template <typename T>
T *AllocateArrayMemoryActual(s64 count)
{
	auto p = malloc(count * sizeof(T)); // @TODO
	return new(p) T[count];
}

void *AllocateMemory(s64 size);
void *AllocateAlignedMemory(s64 alignment, s64 size);
void SetMemory(void *destination, s8 setTo, s64 byteCount);
void CopyMemory(const void *source, void *destination, s64 byteCount);
void MoveMemory(void *source, void *destination, s64 byteCount);
