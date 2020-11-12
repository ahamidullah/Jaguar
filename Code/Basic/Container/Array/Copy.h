#pragma once

// Implicit type conversion does not work with template functions, so we'll have to create overloads of CopyArray for each combination of
// Array, StaticArray, and ArrayView. C++ sucks!

#include "../../Assert.h"

namespace array
{

template <typename T> struct Array;
template <typename T> struct View;

template <typename T>
void Copy(Array<T> src, View<T> dst)
{
	Assert(src.count == dst.count);
	for (auto i = 0; i < src.count; i += 1)
	{
		dst[i] = src[i];
	}
}

template <typename T>
void Copy(Array<T> src, Array<T> dst)
{
	Assert(src.count == dst.count);
	for (auto i = 0; i < src.count; i += 1)
	{
		dst[i] = src[i];
	}
}

template <typename T>
void Copy(View<T> src, Array<T> dst)
{
	Assert(src.count == dst.count);
	for (auto i = 0; i < src.count; i += 1)
	{
		dst[i] = src[i];
	}
}

template <typename T>
void Copy(View<T> src, View<T> dst)
{
	Assert(src.count == dst.count);
	for (auto i = 0; i < src.count; i += 1)
	{
		dst[i] = src[i];
	}
}

}
