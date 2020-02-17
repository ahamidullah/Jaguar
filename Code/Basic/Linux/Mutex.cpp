void CreateMutex(Mutex *mutex) {
	pthread_mutex_init(mutex, NULL);
}

void LockMutex(Mutex *mutex) {
	pthread_mutex_lock(mutex);
}

void UnlockMutex(Mutex *mutex) {
	pthread_mutex_unlock(mutex);
}
