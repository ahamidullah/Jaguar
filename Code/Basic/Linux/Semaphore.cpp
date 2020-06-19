#include "../Basic.h"

Semaphore CreateSemaphore(s64 initialValue)
{
	sem_t semaphore;
	sem_init(&semaphore, 0, initialValue);
	return semaphore;
}

void SignalSemaphore(Semaphore *semaphore)
{
	sem_post(semaphore);
}

void WaitOnSemaphore(Semaphore *semaphore)
{
	sem_wait(semaphore);
}

s64 GetSemaphoreValue(Semaphore *semaphore)
{
	auto value = s32{};
	sem_getvalue(semaphore, &value);
	return value;
}
