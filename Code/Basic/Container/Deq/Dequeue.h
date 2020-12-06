#pragma once

#include "Basic/Container/Array.h"
#include "Basic/Log.h"

s64 DivideAndRoundUp(s64 x, s64 y); // @TODO

namespace deq
{

template <typename T>
struct dequeue
{
	mem::allocator *allocator;
	s64 blockSize;
	arr::array<arr::array<T>> useBlocks;
	arr::array<arr::array<T>> blockPool;
	s64 start;
	s64 end;
	s64 count;

	void PushFront(T e);
	void PushBack(T e);
	T PopFront();
	T PopBack();
};

const auto DefaultBlockSize = 256;

template <typename T>
dequeue<T> NewWithBlockSizeIn(mem::allocator *a, s64 bs, s64 cap)
{
	Assert(bs > 0);
	Assert(cap >= 0);
	auto nBlks = DivideAndRoundUp(cap, bs);
	auto q = dequeue<T>
	{
		.allocator = a,
		.blockSize = bs,
		.useBlocks = arr::NewWithCapacityIn<arr::array<T>>(a, nBlks),
		.blockPool = arr::NewWithCapacityIn<arr::array<T>>(a, nBlks),
	};
	for (auto i = 0; i < nBlks; i += 1)
	{
		q.blockPool.Append(arr::NewIn<T>(a, bs));
	}
	return q;
}

template <typename T>
dequeue<T> NewWithBlockSize(s64 bs, s64 cap)
{
	return NewWithBlockSizeIn<T>(mem::ContextAllocator(), bs, cap);
}

template <typename T>
dequeue<T> New(mem::allocator *a, s64 cap)
{
	return NewWithBlockSizeIn<T>(a, DefaultBlockSize, cap);
}

template <typename T>
dequeue<T> New(s64 cap)
{
	return New<T>(mem::ContextAllocator(), cap);
}

template <typename T>
void dequeue<T>::PushBack(T e)
{
	if (this->blockSize == 0)
	{
		this->blockSize = DefaultBlockSize;
		this->useBlocks.SetAllocator(mem::ContextAllocator());
		this->blockPool.SetAllocator(mem::ContextAllocator());
	}
	if (this->useBlocks.count == 0)
	{
		if (this->blockPool.count > 0)
		{
			this->useBlocks.Append(this->blockPool.Pop());
		}
		else
		{
			this->useBlocks.Append(arr::New<T>(this->allocator, this->blockSize));
		}
		this->start = 0;
		this->end = 0;
	} else if (this->end == this->blockSize)
	{
		if (this->blockPool.count > 0)
		{
			this->useBlocks.Append(this->blockPool.Pop());
		}
		else
		{
			this->useBlocks.Append(arr::New<T>(this->allocator, this->blockSize));
		}
		this->end = 0;
	}
	(*this->useBlocks.Last())[this->end] = e;
	this->end += 1;
	this->count += 1;
}

template <typename T>
void dequeue<T>::PushFront(T e)
{
	if (this->blockSize == 0)
	{
		this->blockSize = DefaultBlockSize;
		this->useBlocks.SetAllocator(mem::ContextAllocator());
		this->blockPool.SetAllocator(mem::ContextAllocator());
	}
	if (this->useBlocks.count == 0)
	{
		if (this->blockPool.count > 0)
		{
			this->useBlocks.Append(this->blockPool.Pop());
		}
		else
		{
			this->useBlocks.Append(arr::NewIn<T>(this->allocator, this->blockSize));
		}
		this->start = this->blockSize - 1;
		this->end = this->blockSize - 1;
	}
	else if (this->start == -1)
	{
		auto a = arr::NewIn<arr::array<T>>(this->allocator, this->useBlocks.count + 1);
		arr::Copy(this->useBlocks.View(0, this->useBlocks.count), a.View(1, this->useBlocks.count + 1));
		this->useBlocks.Free();
		this->useBlocks = a;
		if (this->blockPool.count > 0)
		{
			this->useBlocks[0] = this->blockPool.Pop();
		}
		else
		{
			this->useBlocks[0] = arr::NewIn<T>(this->allocator, this->blockSize);
		}
		this->start = this->blockSize - 1;
	}
	auto i = this->start;
	this->useBlocks[0][this->start] = e;
	this->start -= 1;
	this->count += 1;
}


template <typename T>
T dequeue<T>::PopFront()
{
	if (this->useBlocks.count == 0 || (this->useBlocks.count == 1 && this->start == this->end))
	{
		Abort("Dequeue", "Tried to PopFront() an empty dequeue.");
	}
	auto e = this->useBlocks[0][this->start];
	this->start += 1;
	if (this->start == this->blockSize)
	{
		this->blockPool.Append(this->useBlocks[0]);
		this->useBlocks.OrderedRemove(0);
		this->start = 0;
	}
	this->count -= 1;
	return e;
}

template <typename T>
T dequeue<T>::PopBack()
{
	if (this->useBlocks.count == 0 || (this->useBlocks.count == 1 && this->start == this->end))
	{
		Abort("Dequeue", "Tried to PopBack() an empty dequeue.");
	}
	auto e = (*this->useBlocks.Last())[this->end];
	this->end -= 1;
	if (this->end == -1)
	{
		this->blockPool.Append(this->useBlocks.Pop());
		this->end = this->blockSize - 1;
	}
	this->count -= 1;
	return e;
}

}
