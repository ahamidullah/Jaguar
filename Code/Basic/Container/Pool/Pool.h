#pragma once

#include "Basic/Container/Array.h"

namespace pool
{

template <typename T>
struct Pool
{
	array::Array<T> elements;
	array::Array<T *> freeList;

	void SetAllocator(Memory::Allocator *a);
	T *Get();
	void Release(T *e);
	void GrowIfNecessary(s64 n);
};

template <typename T>
Pool<T> NewIn(Memory::Allocator *a, s64 cap)
{
	auto p = Pool<T>
	{
		.elements = array::NewIn<T>(a, cap),
		.freeList = array::NewWithCapacityIn<T *>(a, cap),
	};
	for (auto &e : p.elements)
	{
		e = T{};
	}
	for (auto i = 0; i < cap; i++)
	{
		p.freeList.Append(&p.elements[i]);
	}
	return p;
}

template <typename T>
Pool<T> New(s64 size)
{
	return NewPoolIn<T>(Memory::ContextAllocator(), size);
}

template <typename T>
void Pool<T>::SetAllocator(Memory::Allocator *a)
{
	this->elements.SetAllocator(a);
	this->freeList.SetAllocator(a);
}

template <typename T>
void Pool<T>::GrowIfNecessary(s64 n)
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
T *Pool<T>::Get()
{
	this->GrowIfNecessary(1);
	return this->freeList.Pop();
}

template <typename T>
void Pool<T>::Release(T *e)
{
	this->freeList.Append(e);
}

}
