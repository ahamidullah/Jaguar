#include "../Mutex.h"

Mutex NewMutex()
{
	auto m = Mutex{};
	pthread_mutex_init(&m.handle, NULL);
	return m;
}

void Mutex::Lock()
{
	pthread_mutex_lock(&this->handle);
}

void Mutex::Unlock()
{
	pthread_mutex_unlock(&this->handle);
}
