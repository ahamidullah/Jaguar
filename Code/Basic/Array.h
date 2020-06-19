#pragma once

template <typename T>
struct Array
{
	s64 count;
	s64 capacity;
	AllocatorInterface allocator;
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
Array<T> NewArray(s64 count)
{
	return
	{
		.count = count,
		.capacity = count,
		.allocator = contextAllocator,
		.elements = (T *)contextAllocator.allocateMemory(contextAllocator.data, count * sizeof(T)),
	};
}

template <typename T>
Array<T> NewArrayIn(s64 count, AllocatorInterface a)
{
	return
	{
		.count = count,
		.capacity = count,
		.allocator = a,
		.elements = (T *)a.allocateMemory(a.data, count * sizeof(T)),
	};
}

template <typename T>
Array<T> NewArrayWithCapacity(s64 count, s64 capacity)
{
	return
	{
		.count = count,
		.capacity = capacity,
		.allocator = contextAllocator,
		.elements = (T *)contextAllocator.allocateMemory(contextAllocator.data, capacity * sizeof(T)),
	};
}

template <typename T>
Array<T> NewArrayWithCapacityIn(s64 count, s64 capacity, AllocatorInterface a)
{
	return
	{
		.count = count,
		.capacity = capacity,
		.allocator = a,
		.elements = (T *)a.allocateMemory(a.data, capacity * sizeof(T)),
	};
}

template <typename T>
Array<T> NewArrayCopy(Array<T> s)
{
	auto result = NewArray<T>(s.count);
	CopyMemory(s.elements, result.elements, s.count * sizeof(T));
	return result;
}

template <typename T>
Array<T> NewArrayCopyIn(Array<T> s, AllocatorInterface a)
{
	auto result = NewArrayIn<T>(s.count, a);
	CopyMemory(s.elements, result.elements, s.count * sizeof(T));
	return result;
}

template <typename T>
Array<T> NewArrayCopyRange(Array<T> s, s64 start, s64 end)
{
	Assert(end >= start);
	auto count = end - start;
	auto result = NewArray<T>(count);
	CopyMemory(&s.elements[start], result.elements, count * sizeof(T));
	return result;
}

template <typename T>
Array<T> NewArrayCopyRangeIn(Array<T> s, s64 start, s64 end, AllocatorInterface a)
{
	Assert(end >= start);
	auto count = end - start;
	auto result = NewArray<T>(count);
	CopyMemory(&s.elements[start], result.elements, count * sizeof(T));
	return result;
}

template <typename T>
Array<T> NewArrayFromData(const T *s, s64 count)
{
	auto result = NewArray<T>(count);
	CopyMemory(s, result.elements, count * sizeof(T));
	return result;
}

template <typename T>
Array<T> NewArrayFromDataIn(const T *s, s64 count, AllocatorInterface a)
{
	auto result = NewArrayIn<T>(count, a);
	CopyMemory(s, result.elements, count * sizeof(T));
	return result;
}

template <typename T>
void ReserveArray(Array<T> *a, s64 reserve)
{
	if (!a->elements)
	{
		a->allocator = contextAllocator;
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
void ResizeArray(Array<T> *a, s64 newCount)
{
	ReserveArray(a, newCount);
	a->count = newCount;
}

template <typename T>
void DoAppendToArray(Array<T> *a, const T &e)
{
	auto ei = a->count;
	ReserveArray(a, a->count + 1);
	Assert(a->count <= a->capacity);
	Assert(ei < a->capacity);
	a->elements[ei] = e;
	a->count += 1;
}

template <typename T>
void AppendToArray(Array<T> *a, const T &e)
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
void AppendDataToArray(Array<T> *d, const T *s, s64 count)
{
	auto oldArrayCount = d->count;
	auto newArrayCount = d->count + count;
	ResizeArray(d, newArrayCount);
	CopyMemory(s, d->elements + oldArrayCount, count * sizeof(T));
}

template <typename T>
void AppendCopyToArray(Array<T> *d, Array<T> s)
{
	AppendDataToArray(d, s.elements, s.count);
}

template <typename T>
void AppendCopyRangeToArray(Array<T> *d, Array<T> s, s64 count)
{
	AppendDataToArray(d, s.elements, count);
}

template <typename T>
void RemoveFromArray(Array<T> *a, s64 i)
{
	if (!a->elements)
	{
		return;
	}
	if (i == a->count - 1)
	{
		a->count -= 1;
		return;
	}
	MoveMemory(&a->elements[i], &a->elements[i + 1], sizeof(T));
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
T *begin(const Array<T> &a)
{
	return &a.elements[0];
}

template <typename T>
T *end(const Array<T> &a)
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
