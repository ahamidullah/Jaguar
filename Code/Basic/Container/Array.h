#pragma once

#include "MemoryAllocatorInterface.h"
#include "Memory.h"
#include "Assert.h"
#include "PCH.h"

template <typename T>
struct Array
{
	s64 count;
	s64 capacity;
	MemoryAllocatorInterface allocator;
	T *elements;

	T &operator[](s64 i);
	T &operator[](s64 i) const;
};

template <typename T>
T &Array<T>::operator[](s64 i)
{
	Assert(i >= 0 && i < count);
	return elements[i];
}

template <typename T>
T &Array<T>::operator[](s64 i) const
{
	Assert(i >= 0 && i < count);
	return elements[i];
}

template <typename T>
bool operator==(const Array<T> &a, const Array<T> &b)
{
	if (a.count != b.count)
	{
		return false;
	}
	for (auto i = 0; i < a.count; i++)
	{
		if (a.elements[i] != b.elements[i])
		{
			return false;
		}
	}
	return true;
}

template <typename T>
bool operator!=(const Array<T> &a, const Array<T> &b)
{
	return !(a == b);
}

template <typename T>
Array<T> CreateArray(s64 count, MemoryAllocatorInterface allocator)
{
	return
	{
		.count = count,
		.capacity = count,
		.allocator = allocator,
		.elements = (T *)AllocateMemory(allocator, count * sizeof(T)),
	};
}

template <typename T>
Array<T> CreateArrayWithCapacity(s64 count, s64 capacity, MemoryAllocatorInterface allocator)
{
	return
	{
		.count = count,
		.capacity = capacity,
		.allocator = allocator,
		.elements = (T *)AllocateMemory(allocator, capacity * sizeof(T)),
	};
}

template <typename T>
Array<T> CreateArrayFromData(const T *source, s64 count, MemoryAllocatorInterface allocator)
{
	auto result = Array<T>
	{
		.count = count,
		.capacity = count,
		.allocator = allocator,
		.elements = (T *)AllocateMemory(allocator, count * sizeof(T)),
	};
	CopyMemory(source, result.elements, count * sizeof(T));
	return result;
}

template <typename T>
void SetMinimumArrayCapacity(Array<T> *a, s64 minimumCapacity)
{
	if (a->capacity >= minimumCapacity)
	{
		return;
	}
	while (a->capacity < minimumCapacity)
	{
		a->capacity = (a->capacity * 2) + 2;
	}
	auto oldElements = a->elements;
	ResizeMemory(a->allocator, a->elements, a->capacity * sizeof(T));
}

template <typename T>
void ResizeArray(Array<T> *a, s64 newCount)
{
	SetMinimumArrayCapacity(a, newCount);
	a->count = newCount;
}

template <typename T>
void DoAppendElement(Array<T> *a, const T &newElement)
{
	auto newElementIndex = a->count;
	SetMinimumArrayCapacity(a, a->count + 1);
	Assert(a->count <= a->capacity);
	Assert(newElementIndex < a->capacity);
	a->elements[newElementIndex] = newElement;
	a->count += 1;
}

template <typename T>
void AppendToArray(Array<T> *a, const T &newElement)
{
	DoAppendElement(a, newElement);
}

// This overload is necessary to allow appending enum values to u32 arrays.
template <typename T>
void AppendToArray(Array<T> *a, u32 newElement)
{
	DoAppendElement(a, newElement);
}

template <typename T>
void AppendDataToArray(Array<T> *destination, const T *source, s64 sourceCount)
{
	auto oldDestinationCount = destination->count;
	auto newDestinationCount = destination->count + sourceCount;
	ResizeArray(destination, newDestinationCount);
	CopyMemory(source, destination->elements + oldDestinationCount, sourceCount * sizeof(T));
}

template <typename T>
void AppendCopyToArray(Array<T> *destination, const Array<T> &source)
{
	Append(destination, source.elements, source.count);
}

template <typename T>
void AppendCopyRangeToArray(Array<T> *destination, const Array<T> &source, s64 sourceCount)
{
	Append(destination, source.elements, sourceCount);
}

template <typename T>
void RemoveArrayElement(Array<T> *a, s64 index)
{
	if (index == a->count - 1)
	{
		a->count--;
		return;
	}
	MoveMemory(&a->elements[index], &a->elements[index + 1], sizeof(T));
	a->count--;
}

template <typename T>
void DestroyArray(Array<T> *a)
{
	FreeMemory(a->allocator, a->elements);
}

template <typename T>
T *begin(const Array<T> &a)
{
	return &a.elements[0];
}

template <typename T>
T *end(const Array<T> &a)
{
	return &a.elements[a.count];
}

template <typename T>
T *begin(Array<T> *a)
{
	return &a->elements[0];
}

template <typename T>
T *end(Array<T> *a)
{
	return &a->elements[a->count];
}
