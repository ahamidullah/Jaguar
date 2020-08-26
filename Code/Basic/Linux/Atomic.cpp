#include "../Atomic.h"

s32 AtomicAdd32(volatile s32 *op, s32 x)
{
	return __sync_add_and_fetch(op, x);
}

s64 AtomicAdd64(volatile s64 *op, s64 x)
{
	return __sync_add_and_fetch(op, x);
}

s32 AtomicFetchAndAdd32(volatile s32 *op, s32 x)
{
	return __sync_fetch_and_add(op, x);
}

s32 AtomicFetchAndAdd64(volatile s64 *op, s64 x)
{
	return __sync_fetch_and_add(op, x);
}

s32 AtomicCompareAndSwap32(volatile s32 *dst, s32 oldVal, s32 newVal)
{
	return __sync_val_compare_and_swap(dst, oldVal, newVal);
}

s64 AtomicCompareAndSwap64(volatile s64 *dst, s64 oldVal, s64 newVal)
{
	return __sync_val_compare_and_swap(dst, oldVal, newVal);
}

void *AtomicCompareAndSwapPointer(void *volatile *dst, void *oldVal, void *newVal)
{
	return __sync_val_compare_and_swap(dst, oldVal, newVal);
}

void *AtomicFetchAndSetPointer(void *volatile *dst, void *val)
{
	return __sync_lock_test_and_set(dst, val);
}
