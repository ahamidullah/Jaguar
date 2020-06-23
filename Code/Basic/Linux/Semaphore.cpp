#include "../Semaphore.h"

Semaphore CreateSemaphore(s64 val)
{
	auto s = sem_t{};
	sem_init(&s, 0, val);
	return s;
}

void SignalSemaphore(Semaphore *s)
{
	sem_post(s);
}

void WaitOnSemaphore(Semaphore *s)
{
	sem_wait(s);
}

s64 GetSemaphoreValue(Semaphore *s)
{
	auto val = s32{};
	sem_getvalue(s, &val);
	return val;
}
