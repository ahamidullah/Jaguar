#pragma once

#define AllocateStruct(type) (type *)AllocateMemory(sizeof(type))
#define AllocateArray(type, count) (type *)AllocateMemory(count * sizeof(type))

void SetMemory(void *destination, s8 setTo, size_t count);
void *AllocateMemory(size_t size);
void CopyMemory(const void *source, void *destination, size_t size);
void MoveMemory(void *source, void *destination, size_t length);
