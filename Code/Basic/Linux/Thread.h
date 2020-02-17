#pragma once

#include <pthread.h>

#define THREAD_LOCAL __thread

typedef pthread_t ThreadHandle;
typedef void *(*ThreadProcedure)(void *);

s32 GetProcessorCount();
ThreadHandle CreateThread(ThreadProcedure procedure, void *parameter);
u32 GetThreadID();
ThreadHandle GetCurrentThread();
void SetThreadProcessorAffinity(ThreadHandle thread, u32 cpuIndex);
