#include "../Mutex.h"

Mutex CreateMutex()
{
	auto m = Mutex{};
	pthread_mutex_init(&m, NULL);
	return m;
}

void LockMutex(Mutex *m)
{
	pthread_mutex_lock(m);
}

void UnlockMutex(Mutex *m)
{
	pthread_mutex_unlock(m);
}
