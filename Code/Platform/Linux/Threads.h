#pragma once

#include <pthread.h>
#include <semaphore.h>

#define THREAD_LOCAL __thread

typedef pthread_t PlatformThreadHandle;
typedef void *(*PlatformThreadProcedure)(void *);
typedef pthread_mutex_t PlatformMutex;
typedef sem_t PlatformSemaphore;

s32 PlatformGetProcessorCount();
PlatformThreadHandle PlatformCreateThread(PlatformThreadProcedure procedure, void *parameter);
void PlatformSetThreadProcessorAffinity(PlatformThreadHandle thread, u32 cpuIndex);
PlatformThreadHandle PlatformGetCurrentThread();
u32 PlatformGetCurrentThreadID();
void PlatformCreateMutex(PlatformMutex *mutex);
void PlatformLockMutex(PlatformMutex *mutex);
void PlatformUnlockMutex(PlatformMutex *mutex);

PlatformSemaphore PlatformCreateSemaphore(u32 initialValue);
void PlatformSignalSemaphore(PlatformSemaphore *semaphore);
void PlatformWaitOnSemaphore(PlatformSemaphore *semaphore);
s32 PlatformGetSemaphoreValue(PlatformSemaphore *semaphore);

s32 PlatformAtomicAddS32(volatile s32 *operand, s32 addend);
s64 PlatformAtomicAddS64(volatile s64 *operand, s64 addend);
s32 PlatformAtomicFetchAndAddS32(volatile s32 *operand, s32 addend);
s32 PlatformAtomicFetchAndAddS64(volatile s64 *operand, s64 addend);
s32 PlatformCompareAndSwapS32(volatile s32 *destination, s32 oldValue, s32 newValue);
s64 PlatformCompareAndSwapS64(volatile s64 *destination, s64 oldValue, s64 newValue);
void *PlatformCompareAndSwapPointers(void *volatile *target, void *oldValue, void *newValue);
void *PlatformFetchAndSetPointer(void *volatile *target, void *value);
