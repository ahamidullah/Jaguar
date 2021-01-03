#pragma once

#include "Basic/Assert.h"

template <typename T> struct array;
template <typename T> struct view;

// Implicit type conversion does not work with template functions, so we'll have to create overloads of CopyArray for each combination of
// array, static array, and array view. C++ sucks!

template <typename T>
void CopyArray(array<T> src, view<T> dst)
{
	Assert(src.count == dst.count);
	for (auto i = 0; i < src.count; i += 1)
	{
		dst[i] = src[i];
	}
}

template <typename T>
void CopyArray(array<T> src, array<T> dst)
{
	Assert(src.count == dst.count);
	for (auto i = 0; i < src.count; i += 1)
	{
		dst[i] = src[i];
	}
}

template <typename T>
void CopyArray(view<T> src, array<T> dst)
{
	Assert(src.count == dst.count);
	for (auto i = 0; i < src.count; i += 1)
	{
		dst[i] = src[i];
	}
}

template <typename T>
void CopyArray(View<T> src, View<T> dst)
{
	Assert(src.count == dst.count);
	for (auto i = 0; i < src.count; i += 1)
	{
		dst[i] = src[i];
	}
}
