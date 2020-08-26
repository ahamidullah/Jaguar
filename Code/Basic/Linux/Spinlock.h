#pragma once

#include "Common.h"

struct Spinlock
{
	s64 handle;

	void Lock();
	void Unlock();
};
