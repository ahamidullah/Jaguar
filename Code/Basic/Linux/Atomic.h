#pragma once

s32 AtomicAddS32(volatile s32 *operand, s32 addend);
s64 AtomicAddS64(volatile s64 *operand, s64 addend);
s32 AtomicFetchAndAddS32(volatile s32 *operand, s32 addend);
s32 AtomicFetchAndAddS64(volatile s64 *operand, s64 addend);
s32 AtomicCompareAndSwapS32(volatile s32 *destination, s32 oldValue, s32 newValue);
s64 AtomicCompareAndSwapS64(volatile s64 *destination, s64 oldValue, s64 newValue);
void *AtomicCompareAndSwap(void *volatile *destination, void *oldValue, void *newValue);
void *AtomicFetchAndSet(void *volatile *target, void *value);
