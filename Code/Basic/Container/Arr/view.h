#pragma once

#include "Find.h"
#include "Copy.h"
#include "../Memory/ContextAllocator.h"
#include "Basic/Assert.h"
#include "Common.h"

template <typename T> struct array;

template <typename T>
struct view
{
	T *elements;
	s64 count;

	T &operator[](s64 i);
	const T &operator[](s64 i) const;
	bool operator==(view<T> a);
	bool operator!=(view<T> a);
	T *begin();
	T *end();
	view<u8> Bytes();
	view<T> View(s64 start, s64 end);
	array<T> Copy();
	array<T> CopyIn(mem::Allocator *a);
	array<T> CopyRange(s64 start, s64 end);
	array<T> CopyRangeIn(mem::Allocator *a, s64 start, s64 end);
	s64 FindFirst(T e);
	s64 FindLast(T e);
};

template <typename T>
view<T> NewView(T *data, s64 count)
{
	return
	{
		.elements = data,
		.count = count,
	};
}

template <typename T>
T &view<T>::operator[](s64 i)
{
	Assert(i >= 0 && i < count);
	return elements[i];
}

template <typename T>
const T &view<T>::operator[](s64 i) const
{
	Assert(i >= 0 && i < count);
	return elements[i];
}

template <typename T>
bool view<T>::operator==(view<T> a)
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
bool view<T>::operator!=(view<T> a)
{
	return !(*this == a);
}

template <typename T>
T *view<T>::begin()
{
	return &this->elements[0];
}

template <typename T>
T *view<T>::end()
{
	return &this->elements[this->count - 1] + 1;
}

template <typename T>
view<u8> view<T>::Bytes()
{
	return
	{
		.elements = (u8 *)this->elements,
		.count = this->count * sizeof(T),
	};
}

template <typename T>
view<T> view<T>::View(s64 start, s64 end)
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
array<T> view<T>::Copy()
{
	return this->CopyIn(mem::ContextAllocator());
}

template <typename T>
array<T> view<T>::CopyIn(mem::Allocator *a)
{
	auto r = New<T>(a, this->count);
	Copy(*this, r);
	return r;
}

template <typename T>
array<T> view<T>::CopyRange(s64 start, s64 end)
{
	this->CopyRangeIn(mem::ContextAllocator(), start, end);
}

template <typename T>
array<T> view<T>::CopyRangeIn(mem::Allocator *a, s64 start, s64 end)
{
	Assert(end >= start);
	auto ar = New<T>(a, end - start);
	Copy(this->View(start, end), ar);
	return ar;
}

template <typename T>
s64 view<T>::FindFirst(T e)
{
	return FindFirst<T>(*this, e);
}

template <typename T>
s64 view<T>::FindLast(T e)
{
	return FindLast<T>(*this, e);
}
