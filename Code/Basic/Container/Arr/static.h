#pragma once

#include "Find.h"
#include "Copy.h"
#include "Common.h"

namespace arr
{

template <typename T, s64 N>
struct fixed
{
	T elements[N];

	operator View<T>();
	T &operator[](s64 i);
	const T &operator[](s64 i) const;
	bool operator==(fixed<T, N> a);
	bool operator!=(fixed<T, N> a);
	T *begin();
	T *end();
	s64 Count();
	view<T> View(s64 start, s64 end);
	view<u8> Bytes();
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
fixed<T, sizeof...(Ts) + 1> MakeFixed(T t, Ts... ts)
{
	return
	{
		t,
		ts...,
	};
}

template <typename T, s64 N>
fixed<T, N>::operator view<T>()
{
	return view<T>
	{
		.elements = this->elements,
		.count = N,
	};
}

template <typename T, s64 N>
T &fixed<T, N>::operator[](s64 i)
{
	Assert(i >= 0 && i < N);
	return this->elements[i];
}

template <typename T, s64 N>
const T &fixed<T, N>::operator[](s64 i) const
{
	Assert(i >= 0 && i < N);
	return this->elements[i];
}

template <typename T, s64 N>
bool fixed<T, N>::operator==(fixed<T, N> a)
{
	if (this->count != a.count)
	{
		return false;
	}
	for (auto i = 0; i < N; i += 1)
	{
		if ((*this)[i] != a[i])
		{
			return false;
		}
	}
	return true;
}

template <typename T, s64 N>
bool fixed<T, N>::operator!=(fixed<T, N> a)
{
	return !(*this == a);
}

template <typename T, s64 N>
T *fixed<T, N>::begin()
{
	return &(*this)[0];
}

template <typename T, s64 N>
T *fixed<T, N>::end()
{
	return &(*this)[N - 1] + 1;
}

template <typename T, s64 N>
s64 fixed<T, N>::Count()
{
	return N;
}

template <typename T, s64 N>
view<T> fixed<T, N>::View(s64 start, s64 end)
{
	Assert(start <= end);
	Assert(start >= 0);
	Assert(end <= this->count);
	return
	{
		.elements = &(*this)[start],
		.count = end - start,
	};
}

template <typename T, s64 N>
view<u8> fixed<T, N>::Bytes()
{
	return
	{
		.elements = (u8 *)this->elements,
		.count = N * sizeof(T),
	};
}

template <typename T, s64 N>
s64 fixed<T, N>::FindFirst(T e)
{
	return FindFirst(*this, e);
}

template <typename T, s64 N>
s64 fixed<T, N>::FindLast(T e)
{
	return FindLast<T>(*this, e);
}

}
