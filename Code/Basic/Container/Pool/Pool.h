#pragma once

#include "Basic/Container/Arr.h"

namespace pool
{

template <typename T>
struct pool
{
	arr::array<T> elements;
	arr::array<T *> freeList;

	void SetAllocator(mem::allocator *a);
	T *Get();
	void Release(T *e);
	void GrowIfNecessary(s64 n);
};

template <typename T>
pool<T> NewIn(mem::allocator *a, s64 cap)
{
	auto p = pool<T>
	{
		.elements = arr::NewIn<T>(a, cap),
		.freeList = arr::NewWithCapacityIn<T *>(a, cap),
	};
	for (auto &e : p.elements)
	{
		e = {};
	}
	for (auto i = 0; i < cap; i++)
	{
		p.freeList.Append(&p.elements[i]);
	}
	return p;
}

template <typename T>
pool<T> New(s64 size)
{
	return NewPoolIn<T>(mem::ContextAllocator(), size);
}

template <typename T>
void pool<T>::SetAllocator(mem::allocator *a)
{
	this->elements.SetAllocator(a);
	this->freeList.SetAllocator(a);
}

template <typename T>
void pool<T>::GrowIfNecessary(s64 n)
{
	if (this->freeList.count >= n)
	{
		return;
	}
	auto start = this->elements.count; 
	if (this->elements.count == 0)
	{
		this->elements.Resize(n);
		this->freeList.Reserve(n);
	}
	else
	{
		auto cap = this->elements.count;
		while (cap < this->elements.count + n)
		{
			cap *= 2;
		}
		this->elements.Resize(cap);
		this->freeList.Reserve(cap);
	}
	for (auto i = start; i < this->elements.count; i++)
	{
		this->elements[i] = T{};
	}
	for (auto i = start; i < this->elements.count; i++)
	{
		this->freeList.Append(&this->elements[i]);
	}
}

template <typename T>
T *pool<T>::Get()
{
	this->GrowIfNecessary(1);
	return this->freeList.Pop();
}

template <typename T>
void pool<T>::Release(T *e)
{
	this->freeList.Append(e);
}

}
