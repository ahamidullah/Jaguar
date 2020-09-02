#pragma once

#include "Common.h"

struct Spinlock
{
	volatile s64 handle;

	void Lock();
	void Unlock();
};
