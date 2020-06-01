#include "../Atomic.h"

s32 AtomicAdd(volatile s32 *operand, s32 addend)
{
	return __sync_add_and_fetch(operand, addend);
}

s64 AtomicAdd(volatile s64 *operand, s64 addend)
{
	return __sync_add_and_fetch(operand, addend);
}

s32 AtomicFetchAndAdd(volatile s32 *operand, s32 addend)
{
	return __sync_fetch_and_add(operand, addend);
}

s32 AtomicFetchAndAdd(volatile s64 *operand, s64 addend)
{
	return __sync_fetch_and_add(operand, addend);
}

s32 AtomicCompareAndSwap(volatile s32 *destination, s32 oldValue, s32 newValue)
{
	return __sync_val_compare_and_swap(destination, oldValue, newValue);
}

s64 AtomicCompareAndSwap(volatile s64 *destination, s64 oldValue, s64 newValue)
{
	return __sync_val_compare_and_swap(destination, oldValue, newValue);
}

void *AtomicCompareAndSwap(void *volatile *destination, void *oldValue, void *newValue)
{
	return __sync_val_compare_and_swap(destination, oldValue, newValue);
}

void *AtomicFetchAndSet(void *volatile *target, void *value)
{
	return __sync_lock_test_and_set(target, value);
}
