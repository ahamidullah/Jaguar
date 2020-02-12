#pragma once


void SetMemory(void *destination, s8 setTo, size_t count);
#define AllocateStructMemory(type) (type *)AllocateMemory(sizeof(type));
void *AllocateMemory(size_t size);
void Copy_Memory(const void *source, void *destination, size_t size);
