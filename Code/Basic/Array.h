#pragma once

#include <stdlib.h> // realloc @TODO

template <typename T>
struct Array
{
	size_t count = 0;
	size_t capacity = 0;
	T *elements = NULL;

	T &operator[](size_t i);
	T &operator[](size_t i) const;
};

template <typename T>
T &Array<T>::operator[](size_t i)
{
	Assert(i < count);
	return elements[i];
}

template <typename T>
T &Array<T>::operator[](size_t i) const
{
	Assert(i < count);
	return elements[i];
}

template <typename T>
bool operator==(const Array<T> &a, const Array<T> &b)
{
	if (a.count != b.count)
	{
		return false;
	}
	for (size_t i = 0; i < a.count; i++)
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
Array<T> CreateArray(size_t count)
{
	return
	{
		.count = count,
		.capacity = count,
		.elements = AllocateArrayMemory(T, count),
	};
}

template <typename T>
Array<T> CreateArray(size_t count, size_t capacity)
{
	return
	{
		.count = count,
		.capacity = capacity,
		.elements = AllocateArrayMemory(T, capacity),
	};
}

template <typename T>
Array<T> CreateArray(size_t count, const T *source)
{
	Array<T> result =
	{
		.count = count,
		.capacity = count,
		.elements = AllocateArrayMemory(T, count),
	};
	CopyMemory(source, result.elements, count * sizeof(T));
	return result;
}

template <typename T>
void SetMinimumArrayCapacity(Array<T> *a, size_t minimumCapacity)
{
	if (a->capacity >= minimumCapacity)
	{
		return;
	}
	while (a->capacity < minimumCapacity)
	{
		a->capacity = (a->capacity * 2) + 2;
	}
	if (!a->elements)
	{
		a->elements = AllocateArrayMemory(T, a->capacity);
	}
	else
	{
		auto oldElements = a->elements;
		a->elements = AllocateArrayMemory(T, a->capacity);
		CopyMemory(oldElements, a->elements, a->count  * sizeof(T));
		free(oldElements); // @TODO
	}
}

template <typename T>
void ResizeArray(Array<T> *a, size_t newCount)
{
	SetMinimumArrayCapacity(a, newCount);
	a->count = newCount;
}

template <typename T>
void ArrayAppend(Array<T> *a, const T &newElement)
{
	size_t newElementIndex = a->count;
	SetMinimumArrayCapacity(a, a->count + 1);
	Assert(a->count <= a->capacity);
	Assert(newElementIndex < a->capacity);
	a->elements[newElementIndex] = newElement;
	a->count += 1;
}

template <typename T>
void ArrayAppend(Array<T> *destination, const T *source, size_t sourceCount)
{
	size_t oldDestinationCount = destination->count;
	size_t newDestinationCount = destination->count + sourceCount;
	Resize(destination, newDestinationCount);
	CopyMemory(source, destination->elements + oldDestinationCount, sourceCount * sizeof(T));
}

template <typename T>
void ArrayAppend(Array<T> *destination, const Array<T> &source)
{
	Append(destination, source.elements, source.count);
}

template <typename T>
void ArrayAppend(Array<T> *destination, const Array<T> &source, size_t sourceCount)
{
	Append(destination, source.elements, sourceCount);
}

template <typename T>
void RemoveArrayElement(Array<T> *a, size_t index)
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
size_t ArrayLength(const Array<T> &a)
{
	return a.count;
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
