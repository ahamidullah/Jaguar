#pragma once

#include "Common.h"

namespace arr
{

template <typename T> struct View;

template <typename T>
s64 FindFirst(View<T> a, T e)
{
	for (auto i = 0; i < a.count; i += 1)
	{
		if (a[i] == e)
		{
			return i;
		}
	}
	return -1;
}

template <typename T>
s64 FindLast(View<T> a, T e)
{
	auto last = -1;
	for (auto i = 0; i < a.count; i += 1)
	{
		if (a[i] == e)
		{
			last = i;
		}
	}
	return last;
}

}
