#pragma once

#include "String.h"

u32 Hash32(u32 u);
u64 Hash64(u64 u);
u64 HashPointer(void *p);
u64 HashString(String s);

struct Hasher32
{
	u32 hash;

	void Add(u32 x);
	u32 Hash();
};

struct Hasher64
{
	u64 hash;

	void Add(u64 x);
	u64 Hash();
};
