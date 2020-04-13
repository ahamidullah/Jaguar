#include <stdlib.h> // malloc @TODO
#include <string.h> // memcpy @TODO

// @TODO: Use non-caching intrinsics?
void SetMemory(void *destination, s8 setTo, size_t count)
{
	auto d = (char *)destination;
	for (auto i = 0; i < count; ++i)
	{
		d[i] = setTo;
	}
}

void *AllocateMemory(size_t size)
{
	return malloc(size); // @TODO
}

void CopyMemory(const void *source, void *destination, size_t size)
{
	memcpy(destination, source, size);
/*
	const char *s = (const char *)source;
	char *d = (char *)destination;
	for (size_t i = 0; i < count; ++i) {
		d[i] = s[i];
	}
*/
}

// Only legal if source and destination are in the same array.
void MoveMemory(void *source, void *destination, size_t length)
{
	auto s = (char *)source;
	auto d = (char *)destination;
	if (s < d)
	{
		for (s += length, d += length; length; --length)
		{
			*--d = *--s;
		}
	}
	else
	{
		while (length--)
		{
			*d++ = *s++;
		}
	}
}
