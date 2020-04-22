#include <stdlib.h> // malloc @TODO
#include <string.h> // memcpy @TODO

// @TODO: Use non-caching intrinsics?
void SetMemory(void *destination, s8 setTo, s64 byteCount)
{
	auto d = (char *)destination;
	for (auto i = 0; i < byteCount; ++i)
	{
		d[i] = setTo;
	}
}

void *AllocateMemory(s64 size)
{
	return malloc(size); // @TODO
}

void CopyMemory(const void *source, void *destination, s64 byteCount)
{
	memcpy(destination, source, byteCount); // @TODO
}

// Only legal if source and destination are in the same array.
void MoveMemory(void *source, void *destination, s64 byteCount)
{
	auto s = (char *)source;
	auto d = (char *)destination;
	if (s < d)
	{
		for (s += byteCount, d += byteCount; byteCount; --byteCount)
		{
			*--d = *--s;
		}
	}
	else
	{
		while (byteCount--)
		{
			*d++ = *s++;
		}
	}
}
