#pragma once

#include <pthread.h>
#include <semaphore.h>

#define THREAD_LOCAL __thread

typedef pthread_t PlatformThreadHandle;
typedef void *(*PlatformThreadProcedure)(void *);
typedef pthread_mutex_t PlatformMutex;
typedef sem_t PlatformSemaphore;
