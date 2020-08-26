#include "../Semaphore.h"

Semaphore NewSemaphore(s64 val)
{
	auto s = Semaphore{};
	sem_init(&s.handle, 0, val);
	return s;
}

void Semaphore::Signal()
{
	sem_post(&this->handle);
}

void Semaphore::Wait()
{
	sem_wait(&this->handle);
}

s64 Semaphore::Value()
{
	auto val = s32{};
	sem_getvalue(&this->handle, &val);
	return val;
}
