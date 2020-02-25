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
Array<T> CreateArray(size_t count, size_t capacity)
{
	return {
		.count = count,
		.capacity = capacity,
		.elements = AllocateArray(T, capacity),
	};
}

template <typename T>
Array<T> CreateArray(size_t count)
{
	return {
		.count = count,
		.capacity = count,
		.elements = AllocateArray(T, count),
	};
}

template<typename T>
size_t ArrayArgumentCount(const T&)
{
    return 1;
}

template<typename T, typename... ElementPack>
size_t ArrayArgumentCount(const T& first, const ElementPack... rest)
{
    return 1 + ArrayArgumentCount(rest...);
}

template<typename T>
void CopyArrayArguments(size_t writeIndex, Array<T> *result, const T &newElement)
{
	result->elements[writeIndex] = newElement;
}

template<typename T, typename... ElementPack>
void CopyArrayArguments(size_t writeIndex, Array<T> *result, const T &first, const ElementPack... rest)
{
	result->elements[writeIndex] = first;
	CopyArrayArguments(writeIndex + 1, result, rest...);
}

template<typename T, typename... ElementPack>
Array<T> CreateInitializedArray(const T &first, const ElementPack... rest)
{
	Array<T> result = CreateArray<T>(ArrayArgumentCount(first, rest...));
	CopyArrayArguments(0, &result, first, rest...);
	return result;
}

template <typename T>
Array<T> CreateArray(size_t dataCount, const T *dataPointer)
{
	Array<T> a = {
		.count = dataCount,
		.capacity = dataCount,
		.elements = AllocateArray(T, dataCount),
	};
	CopyMemory(dataPointer, a.elements, dataCount * sizeof(T));
	return a;
}

template <typename T>
void SetMinimumCapacity(Array<T> *a, size_t minimumCapacity)
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
		a->elements = (T *)malloc(sizeof(T) * a->capacity); // @TODO
	}
	else
	{
		a->elements = (T *)realloc(a->elements, sizeof(T) * a->capacity); // @TODO
	}
}

template <typename T>
void Resize(Array<T> *a, size_t newCount)
{
	SetMinimumCapacity(a, newCount);
	a->count = newCount;
}

// @TODO: Make this variadic and take any number of new elements.
template <typename T>
void Append(Array<T> *a, const T &newElement)
{
	size_t newElementIndex = a->count;
	Resize(a, a->count + 1);
	Assert(a->count <= a->capacity);
	Assert(newElementIndex < a->count);
	a->elements[newElementIndex] = newElement;
}

template <typename T>
void SetSize(Array<T> *a, size_t newCount)
{
	a->count = newCount;
}

template <typename T>
void Append(Array<T> *destination, const T *source, size_t sourceCount)
{
	size_t oldDestinationCount = destination->count;
	size_t newDestinationCount = destination->count + sourceCount;
	Resize(destination, newDestinationCount);
	CopyMemory(source, destination->elements + oldDestinationCount, sourceCount * sizeof(T)); // @TODO
}

template <typename T>
void Append(Array<T> *destination, const Array<T> &source)
{
	Append(destination, source.elements, source.count);
}

template <typename T>
void Append(Array<T> *destination, const Array<T> &source, size_t sourceCount)
{
	Append(destination, source.elements, sourceCount);
}

template <typename T>
void Remove(Array<T> *a, size_t index)
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
size_t Length(const Array<T> &a)
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
