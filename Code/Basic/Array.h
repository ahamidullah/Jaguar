#pragma once

#include "AllocatorInterface.h"
#include "Assert.h"

#include "Code/Common.h"

AllocatorInterface ContextAllocator();
void CopyMemory(const void *src, void *dst, s64 n);
void MoveMemory(void *src, void *dst, s64 n);

template <typename T>
struct ArrayView
{
	T *elements;
	s64 count;

	T &operator[](s64 i);
	const T &operator[](s64 i) const;
};

template <typename T>
T &ArrayView<T>::operator[](s64 i)
{
	Assert(i >= 0 && i < count);
	return elements[i];
}

template <typename T>
const T &ArrayView<T>::operator[](s64 i) const
{
	Assert(i >= 0 && i < count);
	return elements[i];
}

template <typename T>
bool operator==(ArrayView<T> a, ArrayView<T> b)
{
	if (a.count != b.count)
	{
		return false;
	}
	for (auto i = 0; i < a.count; i++)
	{
		if (a[i] != b[i])
		{
			return false;
		}
	}
	return true;
}

template <typename T>
bool operator!=(ArrayView<T> a, ArrayView<T> b)
{
	return !(a == b);
}

template <typename T>
struct Array
{
	AllocatorInterface allocator;
	T *elements;
	s64 count;
	s64 capacity;

	operator ArrayView<T>();
	T &operator[](s64 i);
	const T &operator[](s64 i) const;
};

template <typename T>
Array<T>::operator ArrayView<T>()
{
	return ArrayView<T>
	{
		.elements = elements,
		.count = count, 
	};
}

template <typename T>
T &Array<T>::operator[](s64 i)
{
	Assert(i >= 0 && i < count);
	return elements[i];
}

template <typename T>
const T &Array<T>::operator[](s64 i) const
{
	Assert(i >= 0 && i < count);
	return elements[i];
}

template <typename T>
bool operator==(Array<T> a, Array<T> b)
{
	if (a.count != b.count)
	{
		return false;
	}
	for (auto i = 0; i < a.count; i++)
	{
		if (a[i] != b[i])
		{
			return false;
		}
	}
	return true;
}

template <typename T>
bool operator!=(Array<T> a, Array<T> b)
{
	return !(a == b);
}

template <typename T>
Array<T> NewArray(s64 count)
{
	return
	{
		.allocator = ContextAllocator(),
		.elements = (T *)ContextAllocator().allocateMemory(ContextAllocator().data, count * sizeof(T)),
		.count = count,
		.capacity = count,
	};
}

template <typename T>
Array<T> NewArrayIn(s64 count, AllocatorInterface a)
{
	return
	{
		.allocator = a,
		.elements = (T *)a.allocateMemory(a.data, count * sizeof(T)),
		.count = count,
		.capacity = count,
	};
}

template <typename T>
Array<T> NewArrayWithCapacity(s64 count, s64 capacity)
{
	return
	{
		.allocator = ContextAllocator(),
		.elements = (T *)ContextAllocator().allocateMemory(ContextAllocator().data, capacity * sizeof(T)),
		.count = count,
		.capacity = capacity,
	};
}

template <typename T>
Array<T> NewArrayWithCapacityIn(s64 count, s64 capacity, AllocatorInterface a)
{
	return
	{
		.allocator = a,
		.elements = (T *)a.allocateMemory(a.data, capacity * sizeof(T)),
		.count = count,
		.capacity = capacity,
	};
}

template <typename T>
Array<T> NewArrayCopy(Array<T> src)
{
	auto r = NewArray<T>(src.count);
	CopyMemory(src.elements, r.elements, src.count * sizeof(T));
	return r;
}

template <typename T>
Array<T> NewArrayCopyIn(Array<T> src, AllocatorInterface a)
{
	auto r = NewArrayIn<T>(src.count, a);
	CopyMemory(src.elements, r.elements, src.count * sizeof(T));
	return r;
}

template <typename T>
Array<T> NewArrayCopyRange(Array<T> src, s64 start, s64 end)
{
	Assert(end >= start);
	auto count = end - start;
	auto r = NewArray<T>(count);
	CopyMemory(&src.elements[start], r.elements, count * sizeof(T));
	return r;
}

template <typename T>
Array<T> NewArrayCopyRangeIn(Array<T> src, s64 start, s64 end, AllocatorInterface a)
{
	Assert(end >= start);
	auto count = end - start;
	auto r = NewArray<T>(count);
	CopyMemory(&src.elements[start], r.elements, count * sizeof(T));
	return r;
}

template <typename T>
Array<T> NewArrayFromData(const T *src, s64 count)
{
	auto r = NewArray<T>(count);
	CopyMemory(src, r.elements, count * sizeof(T));
	return r;
}

template <typename T>
Array<T> NewArrayFromDataIn(const T *src, s64 count, AllocatorInterface a)
{
	auto r = NewArrayIn<T>(count, a);
	CopyMemory(src, r.elements, count * sizeof(T));
	return r;
}

template <typename T>
void ReserveArray(Array<T> *a, s64 reserve)
{
	if (!a->elements)
	{
		a->allocator = ContextAllocator();
		a->elements = (T *)a->allocator.allocateMemory(a->allocator.data, reserve * sizeof(T));
		a->capacity = reserve;
		return;
	}
	if (a->capacity >= reserve)
	{
		return;
	}
	while (a->capacity < reserve)
	{
		a->capacity = (a->capacity * 2);
	}
	a->elements = (T *)a->allocator.resizeMemory(a->allocator.data, a->elements, a->capacity * sizeof(T));
}

template <typename T>
void ResizeArray(Array<T> *a, s64 count)
{
	ReserveArray(a, count);
	a->count = count;
}

template <typename T>
void DoAppendToArray(Array<T> *a, T e)
{
	auto i = a->count;
	ReserveArray(a, a->count + 1);
	Assert(a->count <= a->capacity);
	Assert(i < a->capacity);
	a->elements[i] = e;
	a->count += 1;
}

template <typename T>
void AppendToArray(Array<T> *a, T e)
{
	DoAppendToArray(a, e);
}

// This overload is necessary to allow appending enum values to u32 arrays.
// @TODO: Is there a better way of doing this?
template <typename T>
void AppendToArray(Array<T> *a, u32 e)
{
	DoAppendToArray(a, e);
}

template <typename T>
void AppendDataToArray(Array<T> *dst, const T *src, s64 n)
{
	auto oldCount = dst->count;
	auto newCount = dst->count + n;
	ResizeArray(dst, newCount);
	CopyMemory(src, dst->elements + oldCount, n * sizeof(T));
}

template <typename T>
void AppendCopyToArray(Array<T> *dst, Array<T> src)
{
	AppendDataToArray(dst, src.elements, src.count);
}

template <typename T>
void AppendCopyRangeToArray(Array<T> *dst, Array<T> src, s64 n)
{
	AppendDataToArray(dst, src.elements, n);
}

template <typename T>
void OrderedRemoveFromArray(Array<T> *a, s64 i)
{
	Assert(i < a->count);
	MoveMemory(&a->elements[i], &a->elements[i + 1], sizeof(T));
	a->count -= 1;
}

template <typename T>
void UnorderedRemoveFromArray(Array<T> *a, s64 i)
{
	Assert(i < a->count);
	a->elements[i] = a->elements[a->count - 1];
	a->count -= 1;
}

template <typename T>
void FreeArray(Array<T> *a)
{
	a->allocator.freeMemory(a->allocator.data, a->elements);
	a->count = 0;
	a->capacity = 0;
	a->elements = NULL;
}

template <typename T>
T *begin(Array<T> a)
{
	return &a.elements[0];
}

template <typename T>
T *end(Array<T> a)
{
	return &a.elements[a.count - 1] + 1;
}

template <typename T>
T *begin(Array<T> *a)
{
	return &a->elements[0];
}

template <typename T>
T *end(Array<T> *a)
{
	return &a->elements[a->count - 1] + 1;
}

template <typename T, s64 N>
struct StaticArray
{
	T elements[N];

	operator ArrayView<T>();
	T &operator[](s64 i);
	const T &operator[](s64 i) const;
};

template <typename T, s64 N>
StaticArray<T, N>::operator ArrayView<T>()
{
	return ArrayView<T>
	{
		.elements = elements,
		.count = N,
	};
}

template <typename T, s64 N>
T &StaticArray<T, N>::operator[](s64 i)
{
	Assert(i >= 0 && i < N);
	return elements[i];
}

template <typename T, s64 N>
const T &StaticArray<T, N>::operator[](s64 i) const
{
	Assert(i >= 0 && i < N);
	return elements[i];
}

template <typename T, s64 N>
bool operator==(StaticArray<T, N> a, StaticArray<T, N> b)
{
	if (a.count != b.count)
	{
		return false;
	}
	for (auto i = 0; i < N; i++)
	{
		if (a[i] != b[i])
		{
			return false;
		}
	}
	return true;
}

template <typename T, s64 N>
bool operator!=(StaticArray<T, N> a, StaticArray<T, N> b)
{
	return !(a == b);
}

template <typename T, s64 N>
T *begin(StaticArray<T, N> a)
{
	return &a.elements[0];
}

template <typename T, s64 N>
T *end(StaticArray<T, N> a)
{
	return &a.elements[a.count - 1] + 1;
}

template <typename T, s64 N>
T *begin(StaticArray<T, N> *a)
{
	return &a->elements[0];
}

template <typename T, s64 N>
T *end(StaticArray<T, N> *a)
{
	return &a->elements[a->count - 1] + 1;
}
