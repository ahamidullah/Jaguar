#pragma once

#include "Find.h"
#include "Copy.h"
#include "View.h"
#include "../Memory/ContextAllocator.h"
#include "../Assert.h"
#include "Common.h"

namespace arr
{

template <typename T>
struct Array
{
	mem::Allocator *allocator;
	T *elements;
	s64 count;
	s64 capacity;

	operator view<T>();
	T &operator[](s64 i);
	const T &operator[](s64 i) const;
	bool operator==(array<T> a);
	bool operator!=(array<T> a);
	T *begin();
	T *end();
	void Free();
	void SetAllocator(mem::Allocator *a);
	void Reserve(s64 reserve);
	void Resize(s64 count);
	void Append(T e);
	void AppendAll(View<T> a);
	void OrderedRemove(s64 index);
	void UnorderedRemove(s64 index);
	typedef bool (*siftProcedure)(T e);
	view<T> Sift(siftProcedure p);
	array<T> Copy();
	array<T> CopyIn(mem::allocator *a);
	array<T> CopyRange(s64 start, s64 end);
	array<T> CopyRangeIn(mem::allocator *a, s64 start, s64 end);
	view<T> View(s64 start, s64 end);
	view<u8> Bytes();
	s64 FindFirst(T e);
	s64 FindLast(T e);
	T Pop();
	T *Last();
};

template <typename T>
array<T> NewIn(mem::allocator *a, s64 count)
{
	return
	{
		.allocator = a,
		.elements = (T *)a->Allocate(count * sizeof(T)),
		.count = count,
		.capacity = count,
	};
}

template <typename T>
array<T> New(s64 count)
{
	return NewIn<T>(mem::ContextAllocator(), count);
}

template <typename T>
array<T> NewWithCapacityIn(mem::allocator *a, s64 cap)
{
	return
	{
		.allocator = a,
		.elements = (T *)a->Allocate(cap * sizeof(T)),
		.capacity = cap,
	};
}

template <typename T>
Array<T> NewWithCapacity(s64 cap)
{
	return NewWithCapacityIn<T>(mem::ContextAllocator(), cap);
}

template <typename T, typename... Ts>
array<T> MakeIn(mem::allocator *a, Ts... ts)
{
	auto r = NewWithCapacityIn<T>(a, sizeof...(ts));
	(r.Append(ts), ...);
	return r;
}

template <typename T, typename... Ts>
array<T> Make(Ts... ts)
{
	return MakeIn<T>(mem::ContextAllocator(), ts...);
}

template <typename T>
array<T>::operator view<T>()
{
	return View<T>
	{
		.elements = elements,
		.count = count, 
	};
}

template <typename T>
T &array<T>::operator[](s64 i)
{
	Assert(i >= 0 && i < this->count);
	return this->elements[i];
}

template <typename T>
const T &array<T>::operator[](s64 i) const
{
	Assert(i >= 0 && i < this->count);
	return this->elements[i];
}

template <typename T>
bool array<T>::operator==(array<T> a)
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
bool array<T>::operator!=(array<T> a)
{
	return !(*this == a);
}

template <typename T>
T *array<T>::begin()
{
	return &this->elements[0];
}

template <typename T>
T *array<T>::end()
{
	return &this->elements[this->count - 1] + 1;
}

template <typename T>
array<T> array<T>::Copy()
{
	return this->CopyIn(mem::ContextAllocator());
}

template <typename T>
array<T> array<T>::CopyIn(mem::allocator *a)
{
	auto r = NewIn<T>(a, this->count);
	Copy(*this, r);
	return r;
}

template <typename T>
array<T> array<T>::CopyRange(s64 start, s64 end)
{
	return this->CopyRangeIn(mem::ContextAllocator(), start, end);
}

template <typename T>
array<T> array<T>::CopyRangeIn(mem::allocator *a, s64 start, s64 end)
{
	Assert(end >= start);
	auto r = NewIn<T>(a, end - start);
	Copy(this->View(start, end), r);
	return r;
}

template <typename T>
view<u8> array<T>::Bytes()
{
	return
	{
		.elements = (u8 *)this->elements,
		.count = this->count * sizeof(T),
	};
}

template <typename T>
view<T> array<T>::View(s64 start, s64 end)
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
s64 array<T>::FindFirst(T e)
{
	return FindFirst<T>(*this, e);
}

template <typename T>
s64 array<T>::FindLast(T e)
{
	return FindLast<T>(*this, e);
}

template <typename T>
void array<T>::SetAllocator(mem::allocator *a)
{
	this->allocator = a;
}

template <typename T>
void array<T>::Reserve(s64 n)
{
	if (!this->allocator)
	{
		this->allocator = mem::ContextAllocator();
	}
	if (!this->elements)
	{
		this->elements = (T *)this->allocator->Allocate(n * sizeof(T));
		this->capacity = n;
		return;
	}
	if (this->capacity >= n)
	{
		return;
	}
	if (this->capacity == 0)
	{
		this->capacity = 1;
	}
	while (this->capacity < n)
	{
		this->capacity = (this->capacity * 2);
	}
	this->elements = (T *)this->allocator->Resize(this->elements, this->capacity * sizeof(T));
}

template <typename T>
void array<T>::Resize(s64 n)
{
	this->Reserve(n);
	this->count = n;
}

template <typename T>
void array<T>::Append(T e)
{
	auto i = this->count;
	this->Resize(this->count + 1);
	Assert(this->count <= this->capacity);
	Assert(i < this->capacity);
	(*this)[i] = e;
}

template <typename T>
void array<T>::AppendAll(view<T> a)
{
	auto oldCount = this->count;
	auto newCount = this->count + a.count;
	this->Resize(newCount);
	Copy(a, this->View(oldCount, newCount));
}

template <typename T>
void array<T>::OrderedRemove(s64 i)
{
	Assert(i > 0 && i < this->count);
	Copy(this->View(i + 1, this->count), this->View(i, this->count - 1));
	this->count -= 1;
}

template <typename T>
void array<T>::UnorderedRemove(s64 i)
{
	Assert(i > 0 && i < this->count);
	this->elements[i] = this->elements[this->count - 1];
	this->count -= 1;
}

template <typename T>
view<T> array<T>::Sift(siftProcedure p)
{
	auto nValid = 0, nInvalid = 0;
	auto i = 0;
	auto count = this->count;
	while (count > 0)
	{
		if (!p((*this)[i]))
		{
			(*this)[i] = (*this)[this->count - 1 - nInvalid];
			nInvalid += 1;
		}
		else
		{
			nValid += 1;
			i += 1;
		}
		count -= 1;
	}
	return
	{
		.elements = (*this)[0],
		.count = nValid,
	};
}

template <typename T>
void array<T>::Free()
{
	if (!this->elements)
	{
		return;
	}
	this->allocator->Deallocate(this->elements);
	this->count = 0;
	this->capacity = 0;
	this->elements = NULL;
}

template <typename T>
T array<T>::Pop()
{
	Assert(this->count > 0);
	this->Resize(this->count - 1);
	return this->elements[this->count];
}

template <typename T>
T *array<T>::Last()
{
	return &(*this)[this->count - 1];
}

}
