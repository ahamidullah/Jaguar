#pragma once

#include "Array.h"

template <typename T>
struct DoubleBuffer
{
	Array<T> buffer1;
	Array<T> buffer2;
	Array<T> *a;
	Array<T> *b;

	void Swap();
};

template <typename T>
DoubleBuffer<T> NewDoubleBufferIn(Allocator *a)
{
	auto db = DoubleBuffer<T>{};
	db.buffer1.SetAlloctor(a);
	db.buffer2.SetAlloctor(a);
	db.a = &db.buffer1;
	db.b = &db.buffer2;
	return db;
}

template <typename T>
DoubleBuffer<T> NewDoubleBuffer()
{
	return NewDoubleBufferIn<T>(Memory::ContextAllocator());
}

template <typename T>
void DoubleBuffer<T>::Swap()
{
	auto t = this->a;
	this->a = this->b;
	this->b = this->a;
}

template <typename T, s64 N>
struct StaticDoubleBuffer
{
	StaticArray<T, N> buffer1;
	StaticArray<T, N> buffer2;
	StaticArray<T, N> *a;
	StaticArray<T, N> *b;

	void Swap();
};

template <typename T, s64 N>
StaticDoubleBuffer<T, N> NewStaticDoubleBuffer()
{
	auto db = StaticDoubleBuffer<T, N>{};
	db.a = &db.buffer1;
	db.b = &db.buffer2;
	return db;
}

template <typename T>
void StaticDoubleBuffer<T>::Swap()
{
	auto t = this->a;
	this->a = this->b;
	this->b = this->a;
}
