#pragma once

#include "Code/Common.h"

s32 AtomicAdd(volatile s32 *operand, s32 addend);
s64 AtomicAdd(volatile s64 *operand, s64 addend);

s32 AtomicFetchAndAdd(volatile s32 *operand, s32 addend);
s32 AtomicFetchAndAdd(volatile s64 *operand, s64 addend);

s32 AtomicCompareAndSwap(volatile s32 *destination, s32 oldValue, s32 newValue);
s64 AtomicCompareAndSwap(volatile s64 *destination, s64 oldValue, s64 newValue);
void *AtomicCompareAndSwap(void *volatile *destination, void *oldValue, void *newValue);

void *AtomicFetchAndSet(void *volatile *target, void *value);
