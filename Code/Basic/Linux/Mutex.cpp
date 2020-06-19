#include "../Basic.h"

Mutex CreateMutex()
{
	auto mutex = Mutex{};
	pthread_mutex_init(&mutex, NULL);
	return mutex;
}

void LockMutex(Mutex *mutex)
{
	pthread_mutex_lock(mutex);
}

void UnlockMutex(Mutex *mutex)
{
	pthread_mutex_unlock(mutex);
}
