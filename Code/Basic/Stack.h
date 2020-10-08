#pragma once

#include "Array.h"

template <typename T>
struct Stack
{
	Array<T> elements;

	void Push(T e);
	T Pop();
	T Top();
	s64 Count();
	void SetAllocator(Allocator *);
};

template <typename T>
Stack<T> NewStackIn(Allocator *a, s64 cap)
{
	return
	{
		.elements = NewArrayWithCapacityIn<T>(a, cap),
	};
}

template <typename T>
Stack<T> NewStack(s64 cap)
{
	return NewStackIn<T>(ContextAllocator(), cap);
}

template <typename T>
void Stack<T>::Push(T e)
{
	this->elements.Append(e);
}

template <typename T>
T Stack<T>::Pop()
{
	Assert(this->elements.count > 0);
	return this->elements.PopLast();
}

template <typename T>
T Stack<T>::Top()
{
	return *this->elements.Last();
}

template <typename T>
s64 Stack<T>::Count()
{
	return this->elements.count;
}

template <typename T>
void Stack<T>::SetAllocator(Allocator *a)
{
	this->elements.SetAllocator(a);
}

template <typename T, s64 N>
struct StaticStack
{
	T elements[N];
	s64 count;

	void Push(T e);
	T Pop();
	T Top();
	s64 Count();
};

template <typename T, s64 N>
void StaticStack<T, N>::Push(T e)
{
	Assert(this->count < N);
	this->count += 1;
	this->elements[this->count - 1] = e;
}

template <typename T, s64 N>
T StaticStack<T, N>::Pop()
{
	Assert(this->count > 0);
	this->count -= 1;
	return this->elements[this->count];
}

template <typename T, s64 N>
T StaticStack<T, N>::Top()
{
	return this->elements[this->count - 1];
}

template <typename T, s64 N>
s64 StaticStack<T, N>::Count()
{
	return this->count;
}
