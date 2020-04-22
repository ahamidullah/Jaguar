#include <sys/syscall.h>

s64 GetProcessorCount()
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}

ThreadHandle CreateThread(ThreadProcedure procedure, void *parameter)
{
	pthread_attr_t threadAttributes;
	if (pthread_attr_init(&threadAttributes))
	{
		Abort("Failed on pthread_attr_init(): %s.", GetPlatformError());
	}
	ThreadHandle thread;
	if (pthread_create(&thread, &threadAttributes, procedure, parameter))
	{
		Abort("Failed on pthread_create(): %s.", GetPlatformError());
	}
	return thread;
}

s64 GetThreadID()
{
	return syscall(__NR_gettid);
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
		Abort("Failed on pthread_setaffinity_np(): %s.", GetPlatformError());
	}
}
