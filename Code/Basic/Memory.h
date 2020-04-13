#pragma once

#include <new> // placement new
#include <stdlib.h> // malloc @TODO

#define AllocateStructMemory(type) AllocateStructMemoryActual<type>()
#define AllocateArrayMemory(type, count) AllocateArrayMemoryActual<type>(count)

template <typename T>
T *AllocateStructMemory()
{
	auto p = malloc(sizeof(T)); // @TODO
	return new(p) T;
}

template <typename T>
T *AllocateArrayMemory(size_t count)
{
	auto p = malloc(count * sizeof(T)); // @TODO
	return new(p) T[count];
}

void *AllocateMemory(size_t size);
void SetMemory(void *destination, s8 setTo, size_t count);
void CopyMemory(const void *source, void *destination, size_t size);
void MoveMemory(void *source, void *destination, size_t length);
