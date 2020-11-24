#pragma once

#include "Find.h"
#include "Copy.h"
#include "../Memory/ContextAllocator.h"
#include "../Assert.h"
#include "Common.h"

namespace array
{

template <typename T> struct Array;

template <typename T>
struct View
{
	T *elements;
	s64 count;

	T &operator[](s64 i);
	const T &operator[](s64 i) const;
	bool operator==(View<T> a);
	bool operator!=(View<T> a);
	T *begin();
	T *end();
	View<u8> Bytes();
	View<T> ToView(s64 start, s64 end);
	Array<T> Copy();
	Array<T> CopyIn(Memory::Allocator *a);
	Array<T> CopyRange(s64 start, s64 end);
	Array<T> CopyRangeIn(Memory::Allocator *a, s64 start, s64 end);
	s64 FindFirst(T e);
	s64 FindLast(T e);
};

template <typename T>
View<T> NewView(T *data, s64 count)
{
	return
	{
		.elements = data,
		.count = count,
	};
}

template <typename T>
T &View<T>::operator[](s64 i)
{
	Assert(i >= 0 && i < count);
	return elements[i];
}

template <typename T>
const T &View<T>::operator[](s64 i) const
{
	Assert(i >= 0 && i < count);
	return elements[i];
}

template <typename T>
bool View<T>::operator==(View<T> a)
{
	if (this->count != a.count)
	{
		return false;
	}
	for (auto i = 0; i < this->count; i += 1)
	{
		if ((*this)[i] != a[i])
		{
			return false;
		}
	}
	return true;
}

template <typename T>
bool View<T>::operator!=(View<T> a)
{
	return !(*this == a);
}

template <typename T>
T *View<T>::begin()
{
	return &this->elements[0];
}

template <typename T>
T *View<T>::end()
{
	return &this->elements[this->count - 1] + 1;
}

template <typename T>
View<u8> View<T>::Bytes()
{
	return
	{
		.elements = (u8 *)this->elements,
		.count = this->count * sizeof(T),
	};
}

template <typename T>
View<T> View<T>::ToView(s64 start, s64 end)
{
	Assert(start <= end);
	Assert(start >= 0);
	Assert(end <= this->count);
	return
	{
		.elements = &this->elements[start],
		.count = end - start,
	};
}

template <typename T>
Array<T> View<T>::Copy()
{
	return this->CopyIn(Memory::ContextAllocator());
}

template <typename T>
Array<T> View<T>::CopyIn(Memory::Allocator *a)
{
	auto r = New<T>(a, this->count);
	Copy(*this, r);
	return r;
}

template <typename T>
Array<T> View<T>::CopyRange(s64 start, s64 end)
{
	this->CopyRangeIn(Memory::ContextAllocator(), start, end);
}

template <typename T>
Array<T> View<T>::CopyRangeIn(Memory::Allocator *a, s64 start, s64 end)
{
	Assert(end >= start);
	auto ar = New<T>(a, end - start);
	Copy(this->ToView(start, end), ar);
	return ar;
}

template <typename T>
s64 View<T>::FindFirst(T e)
{
	return FindFirst<T>(*this, e);
}

template <typename T>
s64 View<T>::FindLast(T e)
{
	return FindLast<T>(*this, e);
}

}
