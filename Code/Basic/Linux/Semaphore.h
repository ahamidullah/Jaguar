#pragma once

#include <semaphore.h>

typedef sem_t Semaphore;

Semaphore CreateSemaphore(u32 initialValue);
void SignalSemaphore(Semaphore *semaphore);
void WaitOnSemaphore(Semaphore *semaphore);
s32 GetSemaphoreValue(Semaphore *semaphore);

