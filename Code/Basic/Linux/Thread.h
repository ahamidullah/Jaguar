#pragma once

#include <pthread.h>

#define THREAD_LOCAL __thread

typedef pthread_t ThreadHandle;
typedef void *(*ThreadProcedure)(void *);

s64 GetProcessorCount();
ThreadHandle CreateThread(ThreadProcedure procedure, void *parameter);
s64 GetThreadID();
ThreadHandle GetCurrentThread();
void SetThreadProcessorAffinity(ThreadHandle thread, s64 cpuIndex);
