#pragma once

#include "../PCH.h"

#include "Code/Common.h"

typedef sem_t Semaphore;

Semaphore CreateSemaphore(s64 val);
void SignalSemaphore(Semaphore *s);
void WaitOnSemaphore(Semaphore *s);
s64 GetSemaphoreValue(Semaphore *s);
