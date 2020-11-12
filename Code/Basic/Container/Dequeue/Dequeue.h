#pragma once

#include "Basic/Container/Array.h"
#include "../../Log.h"

s64 DivideAndRoundUp(s64 x, s64 y); // @TODO

namespace dequeue
{

template <typename T>
struct Dequeue
{
	Memory::Allocator *allocator;
	s64 blockSize;
	array::Array<array::Array<T>> useBlocks;
	array::Array<array::Array<T>> blockPool;
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
Dequeue<T> NewWithBlockSizeIn(Memory::Allocator *a, s64 bs, s64 cap)
{
	Assert(bs > 0);
	Assert(cap >= 0);
	auto nBlks = DivideAndRoundUp(cap, bs);
	auto q = Dequeue<T>
	{
		.allocator = a,
		.blockSize = bs,
		.useBlocks = array::NewWithCapacityIn<array::Array<T>>(a, nBlks),
		.blockPool = array::NewWithCapacityIn<array::Array<T>>(a, nBlks),
	};
	for (auto i = 0; i < nBlks; i += 1)
	{
		q.blockPool.Append(array::NewIn<T>(a, bs));
	}
	return q;
}

template <typename T>
Dequeue<T> NewWithBlockSize(s64 bs, s64 cap)
{
	return NewWithBlockSizeIn<T>(Memory::ContextAllocator(), bs, cap);
}

template <typename T>
Dequeue<T> New(Memory::Allocator *a, s64 cap)
{
	return NewWithBlockSizeIn<T>(a, DefaultBlockSize, cap);
}

template <typename T>
Dequeue<T> New(s64 cap)
{
	return New<T>(Memory::ContextAllocator(), cap);
}

template <typename T>
void Dequeue<T>::PushBack(T e)
{
	if (this->blockSize == 0)
	{
		this->blockSize = DefaultBlockSize;
		this->useBlocks.SetAllocator(Memory::ContextAllocator());
		this->blockPool.SetAllocator(Memory::ContextAllocator());
	}
	if (this->useBlocks.count == 0)
	{
		if (this->blockPool.count > 0)
		{
			this->useBlocks.Append(this->blockPool.Pop());
		}
		else
		{
			this->useBlocks.Append(array::New<T>(this->allocator, this->blockSize));
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
			this->useBlocks.Append(array::New<T>(this->allocator, this->blockSize));
		}
		this->end = 0;
	}
	(*this->useBlocks.Last())[this->end] = e;
	this->end += 1;
	this->count += 1;
}

template <typename T>
void Dequeue<T>::PushFront(T e)
{
	if (this->blockSize == 0)
	{
		this->blockSize = DefaultBlockSize;
		this->useBlocks.SetAllocator(Memory::ContextAllocator());
		this->blockPool.SetAllocator(Memory::ContextAllocator());
	}
	if (this->useBlocks.count == 0)
	{
		if (this->blockPool.count > 0)
		{
			this->useBlocks.Append(this->blockPool.Pop());
		}
		else
		{
			this->useBlocks.Append(array::NewIn<T>(this->allocator, this->blockSize));
		}
		this->start = this->blockSize - 1;
		this->end = this->blockSize - 1;
	}
	else if (this->start == -1)
	{
		auto a = array::NewIn<array::Array<T>>(this->allocator, this->useBlocks.count + 1);
		array::Copy(this->useBlocks.ToView(0, this->useBlocks.count), a.ToView(1, this->useBlocks.count + 1));
		this->useBlocks.Free();
		this->useBlocks = a;
		if (this->blockPool.count > 0)
		{
			this->useBlocks[0] = this->blockPool.Pop();
		}
		else
		{
			this->useBlocks[0] = array::NewIn<T>(this->allocator, this->blockSize);
		}
		this->start = this->blockSize - 1;
	}
	auto i = this->start;
	this->useBlocks[0][this->start] = e;
	this->start -= 1;
	this->count += 1;
}


template <typename T>
T Dequeue<T>::PopFront()
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
T Dequeue<T>::PopBack()
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
