s32 AtomicAddS32(volatile s32 *operand, s32 addend)
{
	return __sync_add_and_fetch(operand, addend);
}

s64 AtomicAddS64(volatile s64 *operand, s64 addend)
{
	return __sync_add_and_fetch(operand, addend);
}

s32 AtomicFetchAndAddS32(volatile s32 *operand, s32 addend)
{
	return __sync_fetch_and_add(operand, addend);
}

s32 AtomicFetchAndAddS64(volatile s64 *operand, s64 addend)
{
	return __sync_fetch_and_add(operand, addend);
}

s32 AtomicCompareAndSwapS32(volatile s32 *destination, s32 oldValue, s32 newValue)
{
	return __sync_val_compare_and_swap(destination, oldValue, newValue);
}

s64 AtomicCompareAndSwapS64(volatile s64 *destination, s64 oldValue, s64 newValue)
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
