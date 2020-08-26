#pragma once

#include "Common.h"

s32 AtomicAdd32(volatile s32 *op, s32 x);
s64 AtomicAdd64(volatile s64 *op, s64 x);
s32 AtomicFetchAndAdd32(volatile s32 *op, s32 x);
s32 AtomicFetchAndAdd64(volatile s64 *op, s64 x);
s32 AtomicCompareAndSwap32(volatile s32 *dst, s32 oldVal, s32 newVal);
s64 AtomicCompareAndSwap64(volatile s64 *dst, s64 oldVal, s64 newVal);
void *AtomicCompareAndSwapPointer(void *volatile *dst, void *oldVal, void *newVal);
void *AtomicFetchAndSetPointer(void *volatile *dst, void *val);
