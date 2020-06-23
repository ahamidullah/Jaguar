#pragma once

#include "../PCH.h"

#include "Code/Common.h"

typedef sem_t Semaphore;

Semaphore CreateSemaphore(s64 initialValue);
void SignalSemaphore(Semaphore *semaphore);
void WaitOnSemaphore(Semaphore *semaphore);
s64 GetSemaphoreValue(Semaphore *semaphore);