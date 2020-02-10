s32 PlatformGetProcessorCount() {
	return sysconf(_SC_NPROCESSORS_ONLN);
}

PlatformThreadHandle PlatformCreateThread(PlatformThreadProcedure procedure, void *parameter) {
	pthread_attr_t threadAttributes;
	if (pthread_attr_init(&threadAttributes)) {
		Abort("Failed on pthread_attr_init(): %s", PlatformGetError());
	}
	PlatformThreadHandle thread;
	if (pthread_create(&thread, &threadAttributes, procedure, parameter)) {
		Abort("Failed on pthread_create(): %s", PlatformGetError());
	}
	return thread;
}

PlatformThreadHandle PlatformGetCurrentThread() {
	return pthread_self();
}

void PlatformSetThreadProcessorAffinity(PlatformThreadHandle thread, u32 cpuIndex) {
	cpu_set_t cpuSet;
	CPU_ZERO(&cpuSet);
	CPU_SET(cpuIndex, &cpuSet);
	if (pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuSet)) {
		Abort("Failed on pthread_setaffinity_np(): %s", PlatformGetError());
	}
}

u32 PlatformGetCurrentThreadID() {
	return syscall(__NR_gettid);
}

void PlatformCreateMutex(PlatformMutex *mutex) {
	pthread_mutex_init(mutex, NULL);
}

void PlatformLockMutex(PlatformMutex *mutex) {
	pthread_mutex_lock(mutex);
}

void PlatformUnlockMutex(PlatformMutex *mutex) {
	pthread_mutex_unlock(mutex);
}

PlatformSemaphore PlatformCreateSemaphore(u32 initialValue) {
	sem_t semaphore;
	sem_init(&semaphore, 0, initialValue);
	return semaphore;
}

void PlatformSignalSemaphore(PlatformSemaphore *semaphore) {
	sem_post(semaphore);
}

void PlatformWaitOnSemaphore(PlatformSemaphore *semaphore) {
	sem_wait(semaphore);
}

s32 PlatformGetSemaphoreValue(PlatformSemaphore *semaphore) {
	s32 value;
	sem_getvalue(semaphore, &value);
	return value;
}

s32 PlatformAtomicAddS32(volatile s32 *operand, s32 addend) {
	return __sync_add_and_fetch(operand, addend);
}

s64 PlatformAtomicAddS64(volatile s64 *operand, s64 addend) {
	return __sync_add_and_fetch(operand, addend);
}

s32 PlatformAtomicFetchAndAddS32(volatile s32 *operand, s32 addend) {
	return __sync_fetch_and_add(operand, addend);
}

s32 PlatformAtomicFetchAndAddS64(volatile s64 *operand, s64 addend) {
	return __sync_fetch_and_add(operand, addend);
}

s32 PlatformCompareAndSwapS32(volatile s32 *destination, s32 oldValue, s32 newValue) {
	return __sync_val_compare_and_swap(destination, oldValue, newValue);
}

s64 PlatformCompareAndSwapS64(volatile s64 *destination, s64 oldValue, s64 newValue) {
	return __sync_val_compare_and_swap(destination, oldValue, newValue);
}

void *PlatformCompareAndSwapPointers(void *volatile *destination, void *oldValue, void *newValue) {
	return __sync_val_compare_and_swap(destination, oldValue, newValue);
}

void *PlatformFetchAndSetPointer(void *volatile *target, void *value) {
	return __sync_lock_test_and_set(target, value);
}
