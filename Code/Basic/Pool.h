#pragma once

#include "Array.h"
#include "Stack.h"

template <typename T>
struct Pool
{
};

template <typename T, s64 N>
struct FixedPool
{
	StaticArray<T, N> elements;
	StaticStack<T *, N> freeList;

	T &operator[](s64 i);
	T *begin();
	T *end();
	T *Get();
	void Release(T *e);
	s64 Used();
	s64 Available();
};

template <typename T, s64 N>
FixedPool<T, N> NewFixedPool()
{
	auto p = FixedPool<T, N>{};
	for (auto i = 0; i < N; i++)
	{
		p.freeList.Push(&p.elements[i]);
	}
	return p;
}

template <typename T, s64 N>
T &FixedPool<T, N>::operator[](s64 i)
{
	Assert(i >= 0 && i < N);
	return this->elements[i];
}

template <typename T, s64 N>
T *FixedPool<T, N>::begin()
{
	return &this->elements[0];
}

template <typename T, s64 N>
T *FixedPool<T, N>::end()
{
	return &this->elements[N - 1] + 1;
}

template <typename T, s64 N>
T *FixedPool<T, N>::Get()
{
	Assert(this->freeList.Count() > 0);
	return this->freeList.Pop();
}

template <typename T, s64 N>
void FixedPool<T, N>::Release(T *e)
{
	this->freeList.Push(e);
}

template <typename T, s64 N>
s64 FixedPool<T, N>::Used()
{
	return N - freeList.Count();
}

template <typename T, s64 N>
s64 FixedPool<T, N>::Available()
{
	return freeList.Count();
}
