#pragma once

#include "../PCH.h"

#include "Code/Common.h"

#define THREAD_LOCAL __thread

typedef pthread_t ThreadHandle;
typedef void *(*ThreadProcedure)(void *);

s64 GetProcessorCount();
s64 GetThreadID();
ThreadHandle GetCurrentThread();
ThreadHandle CreateThread(ThreadProcedure procedure, void *parameter);
void SetThreadProcessorAffinity(ThreadHandle thread, s64 cpuIndex);
