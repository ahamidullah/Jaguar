#pragma once

#include "../PCH.h"

struct Mutex
{
	pthread_mutex_t handle;

	void Lock();
	void Unlock();
};

Mutex NewMutex();
