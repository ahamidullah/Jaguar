#pragma once

#include <pthread.h>

typedef pthread_mutex_t Mutex;

Mutex CreateMutex();
void LockMutex(Mutex *mutex);
void UnlockMutex(Mutex *mutex);
