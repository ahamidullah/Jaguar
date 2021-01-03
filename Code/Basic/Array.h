#pragma once

#include "Assert.h"
#include "Common.h"
#include "ContextAllocator.h"

template <typename T> struct array;
template <typename T> struct arrayView;

// Implicit type conversion does not work with template functions, so we'll have to create overloads of CopyArray for each combination of array, static array, and array view. C++ sucks!

template <typename T>
void CopyArray(array<T> src, arrayView<T> dst) {
	Assert(src.count == dst.count);
	for (auto i = 0; i < src.count; i += 1) {
		dst[i] = src[i];
	}
}

template <typename T>
void CopyArray(array<T> src, array<T> dst) {
	Assert(src.count == dst.count);
	for (auto i = 0; i < src.count; i += 1) {
		dst[i] = src[i];
	}
}

template <typename T>
void CopyArray(arrayView<T> src, array<T> dst) {
	Assert(src.count == dst.count);
	for (auto i = 0; i < src.count; i += 1) {
		dst[i] = src[i];
	}
}

template <typename T>
void CopyArray(arrayView<T> src, arrayView<T> dst) {
	Assert(src.count == dst.count);
	for (auto i = 0; i < src.count; i += 1) {
		dst[i] = src[i];
	}
}

template <typename T>
s64 FindFirst(arrayView<T> a, T e) {
	for (auto i = 0; i < a.count; i += 1) {
		if (a[i] == e) {
			return i;
		}
	}
	return -1;
}

template <typename T>
s64 FindLast(arrayView<T> a, T e) {
	auto last = -1;
	for (auto i = 0; i < a.count; i += 1) {
		if (a[i] == e) {
			last = i;
		}
	}
	return last;
}

template <typename T>
struct array {
	allocator *allocator;
	T *elements;
	s64 count;
	s64 capacity;

	operator arrayView<T>();
	T &operator[](s64 i);
	const T &operator[](s64 i) const;
	bool operator==(array<T> a);
	bool operator!=(array<T> a);
	T *begin();
	T *end();
	void Free();
	void SetAllocator(allocator *a);
	void Reserve(s64 reserve);
	void Resize(s64 count);
	void Append(T e);
	void AppendAll(arrayView<T> a);
	void OrderedRemove(s64 index);
	void UnorderedRemove(s64 index);
	typedef bool (*siftProcedure)(T e);
	arrayView<T> Sift(siftProcedure p);
	array<T> Copy();
	array<T> CopyIn(allocator *a);
	array<T> CopyRange(s64 start, s64 end);
	array<T> CopyRangeIn(allocator *a, s64 start, s64 end);
	arrayView<T> View(s64 start, s64 end);
	arrayView<u8> Bytes();
	s64 FindFirst(T e);
	s64 FindLast(T e);
	T Pop();
	T *Last();
};

template <typename T>
array<T> NewArrayIn(allocator *a, s64 count) {
	return {
		.allocator = a,
		.elements = (T *)a->Allocate(count * sizeof(T)),
		.count = count,
		.capacity = count,
	};
}

template <typename T>
array<T> NewArray(s64 count) {
	return NewArrayIn<T>(ContextAllocator()(), count);
}

template <typename T>
array<T> NewArrayWithCapacityIn(allocator *a, s64 cap) {
	return {
		.allocator = a,
		.elements = (T *)a->Allocate(cap * sizeof(T)),
		.capacity = cap,
	};
}

template <typename T>
array<T> NewArrayWithCapacity(s64 cap) {
	return NewArrayWithCapacityIn<T>(ContextAllocator(), cap);
}

template <typename T, typename... Ts>
array<T> MakeArrayIn(allocator *a, Ts... ts) {
	auto r = NewArrayWithCapacityIn<T>(a, sizeof...(ts));
	(r.Append(ts), ...);
	return r;
}

template <typename T, typename... Ts>
array<T> MakeArray(Ts... ts) {
	return MakeArrayIn<T>(ContextAllocator(), ts...);
}

template <typename T>
array<T>::operator arrayView<T>() {
	return arrayView<T>{
		.elements = elements,
		.count = count,
	};
}

template <typename T>
T &array<T>::operator[](s64 i) {
	assert(i >= 0 && i < this->count);
	return this->elements[i];
}

template <typename T>
const T &array<T>::operator[](s64 i) const {
	assert(i >= 0 && i < this->count);
	return this->elements[i];
}

template <typename T>
bool array<T>::operator==(array<T> a) {
	if (this->count != a.count) {
		return false;
	}
	for (auto i = 0; i < this->count; i += 1) {
		if ((*this)[i] != a[i]) {
			return false;
		}
	}
	return true;
}

template <typename T>
bool array<T>::operator!=(array<T> a) {
	return !(*this == a);
}

template <typename T>
T *array<T>::begin() {
	return &this->elements[0];
}

template <typename T>
T *array<T>::end() {
	return &this->elements[this->count - 1] + 1;
}

#if 0
template <typename T>
array<T> array<T>::Copy()
{
	return this->CopyIn(ContextAllocator());
}

template <typename T>
array<T> array<T>::CopyIn(allocator *a)
{
	auto r = NewIn<T>(a, this->count);
	Copy(*this, r);
	return r;
}

template <typename T>
array<T> array<T>::CopyRange(s64 start, s64 end)
{
	return this->CopyRangeIn(ContextAllocator(), start, end);
}

template <typename T>
array<T> array<T>::CopyRangeIn(allocator *a, s64 start, s64 end)
{
	Assert(end >= start);
	auto r = NewIn<T>(a, end - start);
	Copy(this->View(start, end), r);
	return r;
}
#endif

template <typename T>
arrayView<u8> array<T>::Bytes() {
	return {
		.elements = (u8 *)this->elements,
		.count = this->count * sizeof(T),
	};
}

template <typename T>
arrayView<T> array<T>::View(s64 start, s64 end) {
	Assert(start <= end);
	Assert(start >= 0);
	Assert(end <= this->count);
	return {
		.elements = &this->elements[start],
		.count = end - start,
	};
}

template <typename T>
s64 array<T>::FindFirst(T e) {
	return FindFirst<T>(*this, e);
}

template <typename T>
s64 array<T>::FindLast(T e) {
	return FindLast<T>(*this, e);
}

template <typename T>
void array<T>::SetAllocator(allocator *a) {
	this->allocator = a;
}

template <typename T>
void array<T>::Reserve(s64 n) {
	if (!this->allocator) {
		this->allocator = ContextAllocator();
	}
	if (!this->elements) {
		this->elements = (T *)this->allocator->Allocate(n * sizeof(T));
		this->capacity = n;
		return;
	}
	if (this->capacity >= n) {
		return;
	}
	if (this->capacity == 0) {
		this->capacity = 1;
	}
	while (this->capacity < n) {
		this->capacity = (this->capacity * 2);
	}
	this->elements = (T *)this->allocator->Resize(this->elements, this->capacity * sizeof(T));
}

template <typename T>
void array<T>::Resize(s64 n) {
	this->Reserve(n);
	this->count = n;
}

template <typename T>
void array<T>::Append(T e) {
	auto i = this->count;
	this->Resize(this->count + 1);
	Assert(this->count <= this->capacity);
	Assert(i < this->capacity);
	(*this)[i] = e;
}

template <typename T>
void array<T>::AppendAll(arrayView<T> a) {
	auto oldCount = this->count;
	auto newCount = this->count + a.count;
	this->Resize(newCount);
	Copy(a, this->View(oldCount, newCount));
}

template <typename T>
void array<T>::OrderedRemove(s64 i) {
	Assert(i > 0 && i < this->count);
	CopyArray(this->View(i + 1, this->count), this->View(i, this->count - 1));
	this->count -= 1;
}

template <typename T>
void array<T>::UnorderedRemove(s64 i) {
	Assert(i > 0 && i < this->count);
	this->elements[i] = this->elements[this->count - 1];
	this->count -= 1;
}

template <typename T>
arrayView<T> array<T>::Sift(siftProcedure p) {
	auto nValid = 0, nInvalid = 0;
	auto i = 0;
	auto count = this->count;
	while (count > 0) {
		if (!p((*this)[i])) {
			(*this)[i] = (*this)[this->count - 1 - nInvalid];
			nInvalid += 1;
		} else {
			nValid += 1;
			i += 1;
		}
		count -= 1;
	}
	return {
		.elements = (*this)[0],
		.count = nValid,
	};
}

template <typename T>
void array<T>::Free() {
	if (!this->elements) {
		return;
	}
	this->allocator->Deallocate(this->elements);
	this->count = 0;
	this->capacity = 0;
	this->elements = NULL;
}

template <typename T>
T array<T>::Pop() {
	Assert(this->count > 0);
	this->Resize(this->count - 1);
	return this->elements[this->count];
}

template <typename T>
T *array<T>::Last() {
	return &(*this)[this->count - 1];
}

template <typename T, s64 N>
struct staticArray {
	T elements[N];

	operator arrayView<T>();
	T &operator[](s64 i);
	const T &operator[](s64 i) const;
	bool operator==(staticArray<T, N> a);
	bool operator!=(staticArray<T, N> a);
	T *begin();
	T *end();
	s64 Count();
	arrayView<T> View(s64 start, s64 end);
	arrayView<u8> Bytes();
	s64 FindFirst(T e);
	s64 FindLast(T e);
};

/*
	THIS SUCKS
		auto vertAttrs = MakeStaticArray(
			VkVertexInputAttributeDescription
			{
				.location = 0,
				.binding = VulkanVertexBufferBindID,
				.format = VK_FORMAT_R32G32B32_SFLOAT,
				.offset = offsetof(Vertex1P1N, position),
			},
			VkVertexInputAttributeDescription
			{
				.location = 1,
				.binding = VulkanVertexBufferBindID,
				.format = VK_FORMAT_R32G32B32_SFLOAT,
				.offset = offsetof(Vertex1P1N, normal),
			});
*/
template <typename T, typename... Ts>
staticArray<T, sizeof...(Ts) + 1> MakeStaticArray(T t, Ts... ts) {
	return {
		t,
		ts...,
	};
}

template <typename T, s64 N>
staticArray<T, N>::operator view<T>() {
	return view<T>{
		.elements = this->elements,
		.count = N,
	};
}

template <typename T, s64 N>
T &staticArray<T, N>::operator[](s64 i) {
	Assert(i >= 0 && i < N);
	return this->elements[i];
}

template <typename T, s64 N>
const T &staticArray<T, N>::operator[](s64 i) const {
	Assert(i >= 0 && i < N);
	return this->elements[i];
}

template <typename T, s64 N>
bool staticArray<T, N>::operator==(staticArray<T, N> a) {
	if (this->count != a.count) {
		return false;
	}
	for (auto i = 0; i < N; i += 1) {
		if ((*this)[i] != a[i]) {
			return false;
		}
	}
	return true;
}

template <typename T, s64 N>
bool staticArray<T, N>::operator!=(staticArray<T, N> a) {
	return !(*this == a);
}

template <typename T, s64 N>
T *staticArray<T, N>::begin() {
	return &(*this)[0];
}

template <typename T, s64 N>
T *staticArray<T, N>::end() {
	return &(*this)[N - 1] + 1;
}

template <typename T, s64 N>
s64 staticArray<T, N>::Count() {
	return N;
}

template <typename T, s64 N>
arrayView<T> staticArray<T, N>::View(s64 start, s64 end) {
	Assert(start <= end);
	Assert(start >= 0);
	Assert(end <= this->count);
	return {
		.elements = &(*this)[start],
		.count = end - start,
	};
}

template <typename T, s64 N>
arrayView<u8> staticArray<T, N>::Bytes() {
	return {
		.elements = (u8 *)this->elements,
		.count = N * sizeof(T),
	};
}

template <typename T, s64 N>
s64 staticArray<T, N>::FindFirst(T e) {
	return FindFirst(*this, e);
}

template <typename T, s64 N>
s64 staticArray<T, N>::FindLast(T e) {
	return FindLast<T>(*this, e);
}

template <typename T>
struct arrayView {
	T *elements;
	s64 count;

	T &operator[](s64 i);
	const T &operator[](s64 i) const;
	bool operator==(arrayView<T> a);
	bool operator!=(arrayView<T> a);
	T *begin();
	T *end();
	arrayView<u8> Bytes();
	arrayView<T> View(s64 start, s64 end);
	array<T> Copy();
	array<T> CopyIn(allocator *a);
	array<T> CopyRange(s64 start, s64 end);
	array<T> CopyRangeIn(allocator *a, s64 start, s64 end);
	s64 FindFirst(T e);
	s64 FindLast(T e);
};

template <typename T>
arrayView<T> NewArrayView(T *data, s64 count) {
	return {
		.elements = data,
		.count = count,
	};
}

template <typename T>
T &arrayView<T>::operator[](s64 i) {
	Assert(i >= 0 && i < count);
	return elements[i];
}

template <typename T>
const T &arrayView<T>::operator[](s64 i) const {
	Assert(i >= 0 && i < count);
	return elements[i];
}

template <typename T>
bool arrayView<T>::operator==(arrayView<T> a) {
	if (this->count != a.count) {
		return false;
	}
	for (auto i = 0; i < this->count; i += 1) {
		if ((*this)[i] != a[i]) {
			return false;
		}
	}
	return true;
}

template <typename T>
bool arrayView<T>::operator!=(arrayView<T> a) {
	return !(*this == a);
}

template <typename T>
T *arrayView<T>::begin() {
	return &this->elements[0];
}

template <typename T>
T *arrayView<T>::end() {
	return &this->elements[this->count - 1] + 1;
}

template <typename T>
arrayView<u8> arrayView<T>::Bytes() {
	return {
		.elements = (u8 *)this->elements,
		.count = this->count * sizeof(T),
	};
}

template <typename T>
arrayView<T> arrayView<T>::View(s64 start, s64 end) {
	Assert(start <= end);
	Assert(start >= 0);
	Assert(end <= this->count);
	return {
		.elements = &this->elements[start],
		.count = end - start,
	};
}

#if 0
template <typename T>
array<T> arrayView<T>::Copy()
{
	return this->CopyIn(ContextAllocator());
}

template <typename T>
array<T> arrayView<T>::CopyIn(allocator *a)
{
	auto r = New<T>(a, this->count);
	Copy(*this, r);
	return r;
}

template <typename T>
array<T> arrayView<T>::CopyRange(s64 start, s64 end)
{
	this->CopyRangeIn(ContextAllocator(), start, end);
}

template <typename T>
array<T> arrayView<T>::CopyRangeIn(allocator *a, s64 start, s64 end)
{
	Assert(end >= start);
	auto ar = New<T>(a, end - start);
	Copy(this->View(start, end), ar);
	return ar;
}
#endif

template <typename T>
s64 arrayView<T>::FindFirst(T e) {
	return FindFirst<T>(*this, e);
}

template <typename T>
s64 arrayView<T>::FindLast(T e) {
	return FindLast<T>(*this, e);
}
