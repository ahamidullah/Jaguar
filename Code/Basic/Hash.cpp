#include "Hash.h"

// Based on Unreal Engine hash functions.

u32 ROTL32(u32 x, s8 r) 
{
	return (x << r) | (x >> (32 - r));;
}

u64 ROTL64(u64 x, s8 r) 
{
	return (x << r) | (x >> (64 - r));;
}

u32 Hash32(u32 x)
{
	x ^= x >> 16;
 	x *= 0x85ebca6b;
 	x ^= x >> 13;
 	x *= 0xc2b2ae35;
 	x ^= x >> 16;
 	return x;
}

u64 Hash64(u64 x)
{
	x ^= x >> 33;
	x *= 0xff51afd7ed558ccdull;
	x ^= x >> 33;
	x *= 0xc4ceb9fe1a85ec53ull;
	x ^= x >> 33;
	return x;
}

u64 HashPointer(void *p)
{
	Assert(sizeof(PointerInt) == 8);
	auto x = (PointerInt)p;
	return Hash64(x);
}

// @TODO: Where did I get this from? Is the Knuth hash better?
u64 HashString(string::String s)
{
    auto x = u64{5381};
    for (auto c : s)
    {
        x = ((x << 5) + x) + c;
	}
    return x;
}

void Hasher32::Add(u32 x)
{
	x *= 0xcc9e2d51;
	x = ROTL32(x, 15);
	x *= 0x1b873593;

	this->hash ^= x;
	this->hash = ROTL32(this->hash, 12);
	this->hash = this->hash * 5 + 0xe6546b64;
}

u32 Hasher32::Hash()
{
	return Hash32(this->hash);
}

void Hasher64::Add(u64 x)
{
	auto c1 = 0x87c37b91114253d5ull;
	auto c2 = 0x4cf5ad432745937full;
	auto b = (u8 *)&x;
	auto k = 0;
	k ^= ((u64)b[7]) << 56;
	k ^= ((u64)b[6]) << 48;
	k ^= ((u64)b[5]) << 40;
	k ^= ((u64)b[4]) << 32;
	k ^= ((u64)b[3]) << 24;
	k ^= ((u64)b[2]) << 16;
	k ^= ((u64)b[1]) << 8;
	k ^= ((u64)b[0]) << 0;
	k *= 0x87c37b91114253d5ull;
	k = ROTL64(k, 31);
	k *= 0x4cf5ad432745937full;
	this->hash ^= k;
}

u64 Hasher64::Hash()
{
  	return Hash64(this->hash);
}
