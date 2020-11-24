#pragma once

#include "Find.h"
#include "Copy.h"
#include "View.h"
#include "../Memory/ContextAllocator.h"
#include "../Assert.h"
#include "Common.h"

namespace array
{

template <typename T>
struct Array
{
	Memory::Allocator *allocator;
	T *elements;
	s64 count;
	s64 capacity;

	operator View<T>();
	T &operator[](s64 i);
	const T &operator[](s64 i) const;
	bool operator==(Array<T> a);
	bool operator!=(Array<T> a);
	T *begin();
	T *end();
	void Free();
	void SetAllocator(Memory::Allocator *a);
	void Reserve(s64 reserve);
	void Resize(s64 count);
	void Append(T e);
	void AppendAll(View<T> a);
	void OrderedRemove(s64 index);
	void UnorderedRemove(s64 index);
	typedef bool (*SiftProcedure)(T e);
	View<T> Sift(SiftProcedure p);
	Array<T> Copy();
	Array<T> CopyIn(Memory::Allocator *a);
	Array<T> CopyRange(s64 start, s64 end);
	Array<T> CopyRangeIn(Memory::Allocator *a, s64 start, s64 end);
	View<T> ToView(s64 start, s64 end);
	View<u8> ToBytes();
	s64 FindFirst(T e);
	s64 FindLast(T e);
	T Pop();
	T *Last();
};

template <typename T>
Array<T> NewIn(Memory::Allocator *a, s64 count)
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
Array<T> New(s64 count)
{
	return NewIn<T>(Memory::ContextAllocator(), count);
}

template <typename T>
Array<T> NewWithCapacityIn(Memory::Allocator *a, s64 cap)
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
	return NewWithCapacityIn<T>(Memory::ContextAllocator(), cap);
}

template <typename T, typename... Ts>
Array<T> MakeIn(Memory::Allocator *a, Ts... ts)
{
	auto r = NewWithCapacityIn<T>(a, sizeof...(ts));
	(r.Append(ts), ...);
	return r;
}

template <typename T, typename... Ts>
Array<T> Make(Ts... ts)
{
	return MakeIn<T>(Memory::ContextAllocator(), ts...);
}

template <typename T>
Array<T>::operator View<T>()
{
	return View<T>
	{
		.elements = elements,
		.count = count, 
	};
}

template <typename T>
T &Array<T>::operator[](s64 i)
{
	Assert(i >= 0 && i < this->count);
	return this->elements[i];
}

template <typename T>
const T &Array<T>::operator[](s64 i) const
{
	Assert(i >= 0 && i < this->count);
	return this->elements[i];
}

template <typename T>
bool Array<T>::operator==(Array<T> a)
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
bool Array<T>::operator!=(Array<T> a)
{
	return !(*this == a);
}

template <typename T>
T *Array<T>::begin()
{
	return &this->elements[0];
}

template <typename T>
T *Array<T>::end()
{
	return &this->elements[this->count - 1] + 1;
}

template <typename T>
Array<T> Array<T>::Copy()
{
	return this->CopyIn(Memory::ContextAllocator());
}

template <typename T>
Array<T> Array<T>::CopyIn(Memory::Allocator *a)
{
	auto r = NewIn<T>(a, this->count);
	array::Copy(*this, r);
	return r;
}

template <typename T>
Array<T> Array<T>::CopyRange(s64 start, s64 end)
{
	return this->CopyRangeIn(Memory::ContextAllocator(), start, end);
}

template <typename T>
Array<T> Array<T>::CopyRangeIn(Memory::Allocator *a, s64 start, s64 end)
{
	Assert(end >= start);
	auto r = NewIn<T>(a, end - start);
	array::Copy(this->ToView(start, end), r);
	return r;
}

template <typename T>
View<u8> Array<T>::ToBytes()
{
	return
	{
		.elements = (u8 *)this->elements,
		.count = this->count * sizeof(T),
	};
}

template <typename T>
View<T> Array<T>::ToView(s64 start, s64 end)
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
s64 Array<T>::FindFirst(T e)
{
	return FindFirst<T>(*this, e);
}

template <typename T>
s64 Array<T>::FindLast(T e)
{
	return FindLast<T>(*this, e);
}

template <typename T>
void Array<T>::SetAllocator(Memory::Allocator *a)
{
	this->allocator = a;
}

template <typename T>
void Array<T>::Reserve(s64 n)
{
	if (!this->allocator)
	{
		this->allocator = Memory::ContextAllocator();
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
void Array<T>::Resize(s64 n)
{
	this->Reserve(n);
	this->count = n;
}

template <typename T>
void Array<T>::Append(T e)
{
	auto i = this->count;
	this->Resize(this->count + 1);
	Assert(this->count <= this->capacity);
	Assert(i < this->capacity);
	(*this)[i] = e;
}

template <typename T>
void Array<T>::AppendAll(View<T> a)
{
	auto oldCount = this->count;
	auto newCount = this->count + a.count;
	this->Resize(newCount);
	array::Copy(a, this->ToView(oldCount, newCount));
}

template <typename T>
void Array<T>::OrderedRemove(s64 i)
{
	Assert(i > 0 && i < this->count);
	array::Copy(this->ToView(i + 1, this->count), this->ToView(i, this->count - 1));
	this->count -= 1;
}

template <typename T>
void Array<T>::UnorderedRemove(s64 i)
{
	Assert(i > 0 && i < this->count);
	this->elements[i] = this->elements[this->count - 1];
	this->count -= 1;
}

template <typename T>
View<T> Array<T>::Sift(SiftProcedure p)
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
void Array<T>::Free()
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
T Array<T>::Pop()
{
	Assert(this->count > 0);
	this->Resize(this->count - 1);
	return this->elements[this->count];
}

template <typename T>
T *Array<T>::Last()
{
	return &(*this)[this->count - 1];
}

}
