#pragma once

#include "../PCH.h"

typedef pthread_mutex_t Mutex;

Mutex CreateMutex();
void LockMutex(Mutex *mutex);
void UnlockMutex(Mutex *mutex);
