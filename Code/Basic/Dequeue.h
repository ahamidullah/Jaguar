#pragma once

#include "Array.h"
#include "Log.h"

s64 DivideAndRoundUp(s64 x, s64 y);
/*
// @TODO: Move this out of here.
s64 DivideAndRoundUp(s64 x, s64 y)
{
	auto result = s64{x / y};
	if (x % y != 0)
	{
		result += 1;
	}
	return result;
}
*/

template <typename T>
struct Dequeue
{
	Allocator *allocator;
	s64 blockSize;
	Array<Array<T>> activeBlocks;
	Array<Array<T>> blockPool;
	s64 start;
	s64 end;
	s64 count;

	void PushFront(T e);
	void PushBack(T e);
	T PopFront();
	T PopBack();
	s64 Count();
};

const auto DefaultDequeueBlockSize = 256;

template <typename T>
Dequeue<T> NewDequeueWithBlockSizeIn(Allocator *a, s64 bs, s64 cap)
{
	Assert(bs > 0);
	Assert(cap >= 0);
	auto nBlks = DivideAndRoundUp(cap, bs);
	auto q = Dequeue<T>
	{
		.allocator = a,
		.blockSize = bs,
		.activeBlocks = NewArrayWithCapacityIn<Array<T>>(a, nBlks),
		.blockPool = NewArrayWithCapacityIn<Array<T>>(a, nBlks),
	};
	for (auto i = 0; i < nBlks; i += 1)
	{
		q.blockPool.Append(NewArrayIn<T>(a, bs));
	}
	return q;
}

template <typename T>
Dequeue<T> NewDequeueWithBlockSize(s64 bs, s64 cap)
{
	return NewDequeueWithBlockSizeIn<T>(ContextAllocator(), bs, cap);
}

template <typename T>
Dequeue<T> NewDequeueIn(Allocator *a, s64 cap)
{
	return NewDequeueWithBlockSizeIn<T>(a, DefaultDequeueBlockSize, cap);
}

template <typename T>
Dequeue<T> NewDequeue(s64 cap)
{
	return NewDequeueIn<T>(ContextAllocator(), cap);
}

template <typename T>
void Dequeue<T>::PushBack(T e)
{
	if (this->blockSize == 0)
	{
		this->blockSize = DefaultDequeueBlockSize;
		this->activeBlocks.SetAllocator(ContextAllocator());
		this->blockPool.SetAllocator(ContextAllocator());
	}
	if (this->activeBlocks.count == 0)
	{
		if (this->blockPool.count > 0)
		{
			this->activeBlocks.Append(this->blockPool.Pop());
		}
		else
		{
			this->activeBlocks.Append(NewArrayIn<T>(this->allocator, this->blockSize));
		}
		this->start = 0;
		this->end = 0;
	} else if (this->end == this->blockSize)
	{
		if (this->blockPool.count > 0)
		{
			this->activeBlocks.Append(this->blockPool.Pop());
		}
		else
		{
			this->activeBlocks.Append(NewArrayIn<T>(this->allocator, this->blockSize));
		}
		this->end = 0;
	}
	(*this->activeBlocks.Last())[this->end] = e;
	this->end += 1;
	this->count += 1;
}

template <typename T>
void Dequeue<T>::PushFront(T e)
{
	if (this->blockSize == 0)
	{
		this->blockSize = DefaultDequeueBlockSize;
		this->activeBlocks.SetAllocator(ContextAllocator());
		this->blockPool.SetAllocator(ContextAllocator());
	}
	if (this->activeBlocks.count == 0)
	{
		if (this->blockPool.count > 0)
		{
			this->activeBlocks.Append(this->blockPool.Pop());
		}
		else
		{
			this->activeBlocks.Append(NewArrayIn<T>(this->allocator, this->blockSize));
		}
		this->start = this->blockSize - 1;
		this->end = this->blockSize - 1;
	}
	else if (this->start == -1)
	{
		auto a = NewArrayIn<Array<T>>(this->allocator, this->activeBlocks.count + 1);
		CopyArray(this->activeBlocks.View(0, this->activeBlocks.count), a.View(1, this->activeBlocks.count + 1));
		// @TODOthis->activeBlocks.Free();
		this->activeBlocks = a;
		if (this->blockPool.count > 0)
		{
			this->activeBlocks[0] = this->blockPool.Pop();
		}
		else
		{
			this->activeBlocks[0] = NewArrayIn<T>(this->allocator, this->blockSize);
		}
		this->start = this->blockSize - 1;
	}
	auto i = this->start;
	this->activeBlocks[0][this->start] = e;
	this->start -= 1;
	this->count += 1;
}


template <typename T>
T Dequeue<T>::PopFront()
{
	if (this->activeBlocks.count == 0 || (this->activeBlocks.count == 1 && this->start == this->end))
	{
		Abort("Dequeue", "Tried to PopFront() an empty dequeue.");
	}
	auto e = this->activeBlocks[0][this->start];
	this->start += 1;
	if (this->start == this->blockSize)
	{
		this->blockPool.Append(this->activeBlocks[0]);
		this->activeBlocks.OrderedRemove(0);
		this->start = 0;
	}
	this->count -= 1;
	return e;
}

template <typename T>
T Dequeue<T>::PopBack()
{
	if (this->activeBlocks.count == 0 || (this->activeBlocks.count == 1 && this->start == this->end))
	{
		Abort("Dequeue", "Tried to PopBack() an empty dequeue.");
	}
	auto e = (*this->activeBlocks.Last())[this->end];
	this->end -= 1;
	if (this->end == -1)
	{
		this->blockPool.Append(this->activeBlocks.Pop());
		this->end = this->blockSize - 1;
	}
	this->count -= 1;
	return e;
}

template <typename T>
s64 Dequeue<T>::Count()
{
	return this->count;
}
