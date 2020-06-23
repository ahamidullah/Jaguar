#include "../Atomic.h"

s32 AtomicAdd(volatile s32 *o, s32 x)
{
	return __sync_add_and_fetch(o, x);
}

s64 AtomicAdd(volatile s64 *o, s64 x)
{
	return __sync_add_and_fetch(o, x);
}

s32 AtomicFetchAndAdd(volatile s32 *o, s32 x)
{
	return __sync_fetch_and_add(o, x);
}

s32 AtomicFetchAndAdd(volatile s64 *o, s64 x)
{
	return __sync_fetch_and_add(o, x);
}

s32 AtomicCompareAndSwap(volatile s32 *dst, s32 oldVal, s32 newVal)
{
	return __sync_val_compare_and_swap(dst, oldVal, newVal);
}

s64 AtomicCompareAndSwap(volatile s64 *dst, s64 oldVal, s64 newVal)
{
	return __sync_val_compare_and_swap(dst, oldVal, newVal);
}

void *AtomicCompareAndSwap(void *volatile *dst, void *oldVal, void *newVal)
{
	return __sync_val_compare_and_swap(dst, oldVal, newVal);
}

void *AtomicFetchAndSet(void *volatile *dst, void *val)
{
	return __sync_lock_test_and_set(dst, val);
}
