#pragma once

#include "Basic/Container/Array.h"

namespace pool
{

template <typename T, s64 N>
struct fixed
{
	arr::fixed<T, N> elements;
	arr::fixed<T *, N> freeList;
	s64 freeListCount;

	T &operator[](s64 i);
	T *begin();
	T *end();
	T *Get();
	void Release(T *e);
	s64 Used();
	s64 Available();
};

template <typename T, s64 N>
fixed<T, N> NewFixed()
{
	auto p = fixed<T, N>{};
	for (auto i = 0; i < N; i += 1)
	{
		p.freeList[p.freeListCount] = &p.elements[i];
		p.freeListCount += 1;
	}
	return p;
}

template <typename T, s64 N>
T &fixed<T, N>::operator[](s64 i)
{
	Assert(i >= 0 && i < N);
	return this->elements[i];
}

template <typename T, s64 N>
T *fixed<T, N>::begin()
{
	return &this->elements[0];
}

template <typename T, s64 N>
T *fixed<T, N>::end()
{
	return &this->elements[N - 1] + 1;
}

template <typename T, s64 N>
T *fixed<T, N>::Get()
{
	Assert(this->freeListCount > 0 && this->freeListCount <= N);
	auto e = this->freeList[this->freeListCount - 1];
	this->freeListCount -= 1;
	return e;
}

template <typename T, s64 N>
void fixed<T, N>::Release(T *e)
{
	Assert(this->freeListCount < N);
	this->freeList[this->freeListCount] = e;
	Assert(e > this->elements.elements && e < this->elements.elements + N);
	this->freeListCount += 1;
}

template <typename T, s64 N>
s64 fixed<T, N>::Used()
{
	return N - this->freeListCount;
}

template <typename T, s64 N>
s64 fixed<T, N>::Available()
{
	return this->freeListCount;
}

}
