#pragma once

s32 AtomicAdd32(volatile s32 *operand, s32 addend);
s64 AtomicAdd64(volatile s64 *operand, s64 addend);
s32 AtomicFetchAndAdd32(volatile s32 *operand, s32 addend);
s32 AtomicFetchAndAdd64(volatile s64 *operand, s64 addend);
s32 AtomicCompareAndSwap32(volatile s32 *destination, s32 oldValue, s32 newValue);
s64 AtomicCompareAndSwap64(volatile s64 *destination, s64 oldValue, s64 newValue);
void *AtomicCompareAndSwapPointer(void *volatile *destination, void *oldValue, void *newValue);
void *AtomicFetchAndSetPointer(void *volatile *target, void *value);
