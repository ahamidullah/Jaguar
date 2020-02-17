#pragma once

#include <pthread.h>

typedef pthread_mutex_t Mutex;

void CreateMutex(Mutex *mutex);
void LockMutex(Mutex *mutex);
void UnlockMutex(Mutex *mutex);
