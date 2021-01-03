#pragma once

#include "Array.h"
#include "Log.h"

template <typename T>
struct deque {
	allocator *allocator;
	s64 elementsPerBlock;
	array<array<T>> useBlocks;
	array<array<T>> blockPool;
	s64 start;
	s64 end;
	s64 count;

	void Initialize();
	void SetAllocator(allocator *a);
	void PushFront(T e);
	void PushBack(T e);
	T PopFront();
	T PopBack();
};

const auto DefaultElementsPerBlock = 16;

template <typename T>
deque<T> NewDequeWithBlockSizeAndCapacityIn(allocator *a, s64 elemsPerBlock, s64 cap) {
	Assert(elemsPerBlock >= 0);
	Assert(cap >= 0);
	auto d = deque<T>{
		.elementsPerBlock = elemsPerBlock,
	};
	d.SetAllocator(a);
	d.Reserve(cap);
	return q;
}

template <typename T>
deque<T> NewDequeWithBlockSizeAndCapacity(s64 elemsPerBlock, s64 cap) {
	return NewDequeWithBlockSizeAndCapacityIn<T>(ContextAllocator(), elemsPerBlock, count, cap);
}

template <typename T>
deque<T> NewDequeWithBlockSizeIn(allocator *a, s64 elemsPerBlock, s64 count) {
	Assert(elemsPerBlock >= 0);
	Assert(count >= 0);
	auto d = deque<T>{
		.elementsPerBlock = elemsPerBlock,
	};
	d.SetAllocator(a);
	d.Resize(count);
	return d;
}

template <typename T>
deque<T> NewDequeWithBlockSize(s64 elemsPerBlock, s64 count) {
	return NewDequeWithBlockSizeIn<T>(ContextAllocator(), elemsPerBlock, count);
}

template <typename T>
deque<T> NewDequeWithCapacityIn(allocator *a, s64 cap) {
	return NewDequeWithBlockSizeAndCapacityIn<T>(a, DefaultElementsPerBlock, cap);
}

template <typename T>
deque<T> NewDequeWithCapacity(s64 cap) {
	return NewDequeWithBlockSizeAndCapacityIn<T>(ContextAllocator(), DefaultElementsPerBlock, cap);
}

template <typename T>
deque<T> NewDequeIn(allocator *a, s64 count) {
	return NewDequeWithBlockSizeIn<T>(a, DefaultElementsPerBlock, count);
}

template <typename T>
deque<T> NewDeque(s64 count) {
	return NewDequeWithBlockSizeIn<T>(ContextAllocator(), DefaultElementsPerBlock, count);
}

template <typename T>
void deque<T>::Initialize() {
	if (!this->allocator) {
		this->SetAllocator(ContextAllocator());
	}
	if (this->elementsPerBlock == 0) {
		this->elementsPerBlock = DefaultElementsPerBlock;
	}
}

template <typename T>
void deque<T>::SetAllocator(allocator *a) {
	this->allocator = a;
	this->useBlocks.SetAllocator(a);
	this->blockPool.SetAllocator(a);
}

template <typename T>
void deque<T>::PushBack(T e) {
	this->Initialize();
	if (this->useBlocks.count == 0) {
		if (this->blockPool.count > 0) {
			this->useBlocks.Append(this->blockPool.Pop());
		} else {
			this->useBlocks.Append(NewArray<T>(this->allocator, this->elementsPerBlock));
		}
		this->start = 0;
		this->end = 0;
	} else if (this->end == this->elementsPerBlock) {
		if (this->blockPool.count > 0) {
			this->useBlocks.Append(this->blockPool.Pop());
		} else {
			this->useBlocks.Append(NewArray<T>(this->allocator, this->elementsPerBlock));
		}
		this->end = 0;
	}
	(*this->useBlocks.Last())[this->end] = e;
	this->end += 1;
	this->count += 1;
}

template <typename T>
void deque<T>::PushFront(T e) {
	if (this->elementsPerBlock == 0) {
		this->elementsPerBlock = DefaultBlockSize;
		this->useBlocks.SetAllocator(ContextAllocator());
		this->blockPool.SetAllocator(ContextAllocator());
	}
	if (this->useBlocks.count == 0) {
		if (this->blockPool.count > 0) {
			this->useBlocks.Append(this->blockPool.Pop());
		} else {
			this->useBlocks.Append(NewArrayIn<T>(this->allocator, this->elementsPerBlock));
		}
		this->start = this->elementsPerBlock - 1;
		this->end = this->elementsPerBlock - 1;
	} else if (this->start == -1) {
		auto a = NewArrayIn<array<T>>(this->allocator, this->useBlocks.count + 1);
		CopyArray(this->useBlocks.View(0, this->useBlocks.count), a.View(1, this->useBlocks.count + 1));
		this->useBlocks.Free();
		this->useBlocks = a;
		if (this->blockPool.count > 0) {
			this->useBlocks[0] = this->blockPool.Pop();
		} else {
			this->useBlocks[0] = NewArrayIn<T>(this->allocator, this->elementsPerBlock);
		}
		this->start = this->elementsPerBlock - 1;
	}
	auto i = this->start;
	this->useBlocks[0][this->start] = e;
	this->start -= 1;
	this->count += 1;
}

template <typename T>
T deque<T>::PopFront() {
	this->Initialize();
	if (this->useBlocks.count == 0 || (this->useBlocks.count == 1 && this->start == this->end)) {
		Abort("Deque", "Tried to PopFront() an empty deque.");
	}
	auto e = this->useBlocks[0][this->start];
	this->start += 1;
	if (this->start == this->elementsPerBlock) {
		this->blockPool.Append(this->useBlocks[0]);
		this->useBlocks.OrderedRemove(0);
		this->start = 0;
	}
	this->count -= 1;
	return e;
}

template <typename T>
T deque<T>::PopBack() {
	this->Initialize();
	if (this->useBlocks.count == 0 || (this->useBlocks.count == 1 && this->start == this->end)) {
		Abort("Deque", "Tried to PopBack() an empty deque.");
	}
	auto e = (*this->useBlocks.Last())[this->end];
	this->end -= 1;
	if (this->end == -1) {
		this->blockPool.Append(this->useBlocks.Pop());
		this->end = this->elementsPerBlock - 1;
	}
	this->count -= 1;
	return e;
}

template <typename T>
void deque<T>::Resize(s64 count) {
	this->Initialize();
	auto nBlks = DivideAndRoundUp(count, this->elementsPerBlock);
	this->blockPool.Reserve(nBlks);
	this->useBlocks.Reserve(nBlks);
	for (auto i = 0; i < nBlks; i += 1) {
		this->useBlocks.Append(NewArrayIn<T>(this->allocator, this->elementsPerBlock));
	}
}

template <typename T>
void deque<T>::Reserve(s64 cap) {
	this->Initialize();
	auto nBlks = DivideAndRoundUp(cap, this->elementsPerBlock);
	this->blockPool.Reserve(nBlks);
	this->useBlocks.Reserve(nBlks);
	for (auto i = 0; i < nBlks; i += 1) {
		this->blockPool.Append(NewArrayIn<T>(this->allocator, this->elementsPerBlock));
	}
}
