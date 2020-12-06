#pragma once

#include "Basic/Container/Array.h"

namespace pool
{

template <typename T>
struct framePool
{
	arr::array<T> elements;
	s64 frontier;

	void GrowIfNecessary();
	T *Get();
	T GetValue();
	void Reset();
};

template <typename T>
void framePool<T>::GrowIfNecessary()
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
T *framePool<T>::Get()
{
	this->GrowIfNecessary();
	auto i = this->frontier;
	this->frontier += 1;
	return &this->elements[i];
}

template <typename T>
T framePool<T>::GetValue()
{
	this->GrowIfNecessary();
	auto i = this->frontier;
	this->frontier += 1;
	return this->elements[i];
}

template <typename T>
void framePool<T>::Reset()
{
	this->frontier = 0;
}

}
