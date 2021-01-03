#pragma once

template <typename T>
struct pool {
	array<T> elements;
	array<T *> freeList;

	void SetAllocator(allocator *a);
	T *Get();
	void Release(T *e);
	void GrowIfNecessary(s64 n);
};

template <typename T>
pool<T> NewPoolIn(allocator *a, s64 cap) {
	auto p = pool<T>{
		.elements = NewArrayIn<T>(a, cap),
		.freeList = NewArrayWithCapacityIn<T *>(a, cap),
	};
	for (auto &e : p.elements) {
		e = {};
	}
	for (auto i = 0; i < cap; i++) {
		p.freeList.Append(&p.elements[i]);
	}
	return p;
}

template <typename T>
pool<T> NewPool(s64 count) {
	return NewPoolIn<T>(ContextAllocator(), count);
}

template <typename T>
void pool<T>::SetAllocator(allocator *a) {
	this->elements.SetAllocator(a);
	this->freeList.SetAllocator(a);
}

template <typename T>
void pool<T>::GrowIfNecessary(s64 n) {
	if (this->freeList.count >= n) {
		return;
	}
	auto start = this->elements.count;
	if (this->elements.count == 0) {
		this->elements.Resize(n);
		this->freeList.Reserve(n);
	} else {
		auto cap = this->elements.count;
		while (cap < this->elements.count + n) {
			cap *= 2;
		}
		this->elements.Resize(cap);
		this->freeList.Reserve(cap);
	}
	for (auto i = start; i < this->elements.count; i++) {
		this->elements[i] = T{};
	}
	for (auto i = start; i < this->elements.count; i++) {
		this->freeList.Append(&this->elements[i]);
	}
}

template <typename T>
T *pool<T>::Get() {
	this->GrowIfNecessary(1);
	return this->freeList.Pop();
}

template <typename T>
void pool<T>::Release(T *e) {
	this->freeList.Append(e);
}

template <typename T>
struct framePool {
	array<T> elements;
	s64 frontier;

	void GrowIfNecessary();
	T *Get();
	T GetValue();
	void Reset();
};

template <typename T>
framePool<T> NewFramePool(s64 count) {
	return {
		.elements = NewArray<T>(count),
	};
}

template <typename T>
framePool<T> NewFramePoolIn(allocator *a, s64 count) {
	return {
		.elements = NewArrayIn<T>(a, count),
	};
}

template <typename T>
framePool<T> NewFramePoolWithCapacity(s64 cap) {
	return {
		.elements = NewArrayWithCapacity<T>(cap),
	};
}

template <typename T>
framePool<T> NewFramePoolWithCapacityIn(allocator *a, s64 cap) {
	return {
		.elements = NewArrayWithCapacityIn<T>(a, cap),
	};
}

template <typename T>
void framePool<T>::GrowIfNecessary() {
	if (this->frontier < this->elements.count) {
		return;
	}
	auto start = this->elements.count;
	if (this->elements.count == 0) {
		this->elements.Resize(2);
	} else {
		this->elements.Resize(this->elements.count * 2);
	}
	for (auto i = 0; i < start; i++) {
		this->elements[i] = T{};
	}
	this->frontier = start;
}

template <typename T>
T *framePool<T>::Get() {
	this->GrowIfNecessary();
	auto i = this->frontier;
	this->frontier += 1;
	return &this->elements[i];
}

template <typename T>
T framePool<T>::GetValue() {
	this->GrowIfNecessary();
	auto i = this->frontier;
	this->frontier += 1;
	return this->elements[i];
}

template <typename T>
void framePool<T>::Reset() {
	this->frontier = 0;
}

template <typename T, s64 N>
struct staticPool {
	staticArray<T, N> elements;
	staticArray<T *, N> freeList;
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
staticPool<T, N> NewStaticPool() {
	auto p = fixed<T, N>{};
	for (auto i = 0; i < N; i += 1) {
		p.freeList[p.freeListCount] = &p.elements[i];
		p.freeListCount += 1;
	}
	return p;
}

template <typename T, s64 N>
T &staticPool<T, N>::operator[](s64 i) {
	Assert(i >= 0 && i < N);
	return this->elements[i];
}

template <typename T, s64 N>
T *staticPool<T, N>::begin() {
	return &this->elements[0];
}

template <typename T, s64 N>
T *staticPool<T, N>::end() {
	return &this->elements[N - 1] + 1;
}

template <typename T, s64 N>
T *staticPool<T, N>::Get() {
	Assert(this->freeListCount > 0 && this->freeListCount <= N);
	auto e = this->freeList[this->freeListCount - 1];
	this->freeListCount -= 1;
	return e;
}

template <typename T, s64 N>
void staticPool<T, N>::Release(T *e) {
	Assert(this->freeListCount < N);
	this->freeList[this->freeListCount] = e;
	Assert(e > this->elements.elements && e < this->elements.elements + N);
	this->freeListCount += 1;
}

template <typename T, s64 N>
s64 staticPool<T, N>::Used() {
	return N - this->freeListCount;
}

template <typename T, s64 N>
s64 staticPool<T, N>::Available() {
	return this->freeListCount;
}
