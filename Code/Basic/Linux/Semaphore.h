#pragma once

#include "../PCH.h"
#include "Common.h"

struct Semaphore
{
	sem_t handle;

	void Signal();
	void Wait();
	s64 Value();
};

Semaphore NewSemaphore(s64 val);
