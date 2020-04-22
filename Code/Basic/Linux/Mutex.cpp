Mutex CreateMutex()
{
	Mutex m;
	pthread_mutex_init(&m, NULL);
	return m;
}

void LockMutex(Mutex *mutex)
{
	pthread_mutex_lock(mutex);
}

void UnlockMutex(Mutex *mutex)
{
	pthread_mutex_unlock(mutex);
}
