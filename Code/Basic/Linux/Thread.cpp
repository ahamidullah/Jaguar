#include "../Basic.h"

ThreadHandle CreateThread(ThreadProcedure procedure, void *parameter)
{
	pthread_attr_t threadAttributes;
	if (pthread_attr_init(&threadAttributes))
	{
		Abort("Failed on pthread_attr_init(): %k.", GetPlatformError());
	}
	ThreadHandle thread;
	if (pthread_create(&thread, &threadAttributes, procedure, parameter))
	{
		Abort("Failed on pthread_create(): %k.", GetPlatformError());
	}
	return thread;
}

ThreadHandle GetCurrentThread()
{
	return pthread_self();
}

void SetThreadProcessorAffinity(ThreadHandle thread, s64 cpuIndex)
{
	cpu_set_t cpuSet;
	CPU_ZERO(&cpuSet);
	CPU_SET(cpuIndex, &cpuSet);
	if (pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuSet))
	{
		Abort("Failed on pthread_setaffinity_np(): %k.", GetPlatformError());
	}
}

s64 GetThreadID()
{
	return syscall(__NR_gettid);
}

void AcquireSpinLock(SpinLock *lock)
{
	for (;;)
	{
		if (AtomicCompareAndSwap(lock, 0, 1) == 1)
		{
			return;
		}
		while (*lock != 0)
		{
			CPUHintSpinWaitLoop();
		}
	}
}

void ReleaseSpinLock(SpinLock *lock)
{
	Assert(*lock == 1);
	*lock = 0;
}
