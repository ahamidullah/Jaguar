#pragma once

#include "../PCH.h"

#include "Code/Common.h"

#define THREAD_LOCAL __thread

typedef pthread_t ThreadHandle;
typedef void *(*ThreadProcedure)(void *);

ThreadHandle CreateThread(ThreadProcedure procedure, void *parameter);
ThreadHandle GetCurrentThread();
void SetThreadProcessorAffinity(ThreadHandle thread, s64 cpuIndex);
s64 GetThreadID();

typedef u8 SpinLock;

void AcquireSpinLock(SpinLock *lock);
void ReleaseSpinLock(SpinLock *lock);
