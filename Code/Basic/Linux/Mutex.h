#pragma once

#include "../PCH.h"

typedef pthread_mutex_t Mutex;

Mutex CreateMutex();
void LockMutex(Mutex *m);
void UnlockMutex(Mutex *m);
