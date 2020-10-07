#pragma once

#include "Array.h"
#include "Stack.h"

template <typename T>
struct Pool
{
	Array<T> elements;
	Array<T *> freeList;

	void SetAllocator(Allocator *a);
	T *Get();
	void Release(T *e);
	void GrowIfNecessary(s64 n);
};

template <typename T>
Pool<T> NewPoolIn(Allocator *a, s64 cap)
{
	auto p = Pool<T>
	{
		.elements = NewArrayIn<T>(a, cap),
		.freeList = NewArrayWithCapacityIn<T>(a, cap, 0),
	};
	for (auto &e : p.elements)
	{
		e = T{};
	}
	for (auto i = 0; i < cap; i++)
	{
		p.freeList[i].Append(&p.elements[i]);
	}
	return p;
}

template <typename T>
Pool<T> NewPool(s64 size)
{
	return NewPoolIn<T>(ContextAllocator(), size);
}

template <typename T>
void Pool<T>::SetAllocator(Allocator *a)
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
	return this->freeList.PopBack();
}

template <typename T>
void Pool<T>::Release(T *e)
{
	this->freeList.Append(e);
}

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

template <typename T>
struct ValuePool
{
	Array<T> elements;

	T Get();
	void Release(T e);
	void ReleaseAll(ArrayView<T> es);
};

template <typename T>
ValuePool<T> NewValuePoolIn(Allocator *a, s64 cap)
{
	return
	{
		.elements = NewArrayWithCapacityIn<T>(a, cap),
	};
};

template <typename T>
ValuePool<T> NewValuePool(s64 cap)
{
	return NewValuePoolIn<T>(ContextAllocator(), cap);
}

template <typename T>
T ValuePool<T>::Get()
{
	if (this->elements.count > 0)
	{
		return this->elements.PopBack();
	}
	return T{};
}

template <typename T>
void ValuePool<T>::Release(T e)
{
	this->elements.Append(e);
}

template <typename T>
void ValuePool<T>::ReleaseAll(ArrayView<T> es)
{
	this->elements.AppendAll(es);
}

template <typename T>
struct FramePool
{
	Array<T> elements;
	s64 frontier;

	void GrowIfNecessary();
	T *Get();
	T GetValue();
	void Reset();
};

template <typename T>
void FramePool<T>::GrowIfNecessary()
{
	if (this->frontier < this->elements.count)
	{
		return;
	}
	auto start = this->elements.count;
	if (this->elements.count == 0)
	{
		this->elements.Resize(2);
	}
	else
	{
		this->elements.Resize(this->elements.count * 2);
	}
	for (auto i = 0; i < start; i++)
	{
		this->elements[i] = T{};
	}
	this->frontier = start;
}

template <typename T>
T *FramePool<T>::Get()
{
	this->GrowIfNecessary();
	auto i = this->frontier;
	this->frontier += 1;
	return &this->elements[i];
}

template <typename T>
T FramePool<T>::GetValue()
{
	this->GrowIfNecessary();
	auto i = this->frontier;
	this->frontier += 1;
	return this->elements[i];
}

template <typename T>
void FramePool<T>::Reset()
{
	this->frontier = 0;
}
