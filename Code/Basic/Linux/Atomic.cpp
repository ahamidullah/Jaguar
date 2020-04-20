s32 AtomicAdd32(volatile s32 *operand, s32 addend)
{
	return __sync_add_and_fetch(operand, addend);
}

s64 AtomicAdd64(volatile s64 *operand, s64 addend)
{
	return __sync_add_and_fetch(operand, addend);
}

s32 AtomicFetchAndAdd32(volatile s32 *operand, s32 addend)
{
	return __sync_fetch_and_add(operand, addend);
}

s32 AtomicFetchAndAdd64(volatile s64 *operand, s64 addend)
{
	return __sync_fetch_and_add(operand, addend);
}

s32 AtomicCompareAndSwap32(volatile s32 *destination, s32 oldValue, s32 newValue)
{
	return __sync_val_compare_and_swap(destination, oldValue, newValue);
}

s64 AtomicCompareAndSwap64(volatile s64 *destination, s64 oldValue, s64 newValue)
{
	return __sync_val_compare_and_swap(destination, oldValue, newValue);
}

void *AtomicCompareAndSwapPointer(void *volatile *destination, void *oldValue, void *newValue)
{
	return __sync_val_compare_and_swap(destination, oldValue, newValue);
}

void *AtomicFetchAndSetPointer(void *volatile *target, void *value)
{
	return __sync_lock_test_and_set(target, value);
}
