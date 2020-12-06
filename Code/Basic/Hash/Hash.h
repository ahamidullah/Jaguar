#pragma once

#include "Basic/Str.h"

namespace hash
{

u32 U32(u32 u);
u64 U32(u64 u);
u64 Pointer(void *p);
u64 String(str::String s);

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

}
