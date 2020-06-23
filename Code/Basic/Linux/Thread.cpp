#include "../Thread.h"
#include "../Log.h"
#include "../Process.h"
#include "../String.h"
#include "../Memory.h"

Thread NewThread(ThreadProcedure proc, void *param)
{
	auto attrs = pthread_attr_t{};
	if (pthread_attr_init(&attrs))
	{
		Abort("Failed on pthread_attr_init(): %k.", GetPlatformError());
	}
	auto t = Thread{};
	if (pthread_create(&t, &attrs, proc, param))
	{
		Abort("Failed on pthread_create(): %k.", GetPlatformError());
	}
	return t;
}

const auto MaxThreadNameLength = 15;

void SetCurrentThreadName(String n)
{
	if (n.length > MaxThreadNameLength)
	{
		LogPrint(LogLevelError, "Thread", "Failed to set name for thread %k: name is too long (max: %d).\n", n, MaxThreadNameLength);
		return;
	}
	if (prctl(PR_SET_NAME, &n[0], 0, 0, 0) != 0)
	{
		LogPrint(LogLevelError, "Thread", "Failed to set name for thread %k: %k.\n", n, GetPlatformError());
	}
}

String GetCurrentThreadName()
{
	auto buf = (char *)AllocateMemory(MaxThreadNameLength);
	if (prctl(PR_GET_NAME, buf, 0, 0, 0) != 0)
	{
		LogPrint(LogLevelError, "Thread", "Failed to get thread name: %k.\n", GetPlatformError());
	}
	return String{buf};
}

Thread CurrentThread()
{
	return pthread_self();
}

void SetThreadProcessorAffinity(Thread t, s64 cpuIndex)
{
	auto cs = cpu_set_t{};
	CPU_ZERO(&cs);
	CPU_SET(cpuIndex, &cs);
	if (pthread_setaffinity_np(t, sizeof(cs), &cs))
	{
		Abort("Failed on pthread_setaffinity_np(): %k.", GetPlatformError());
	}
}

s64 GetThreadID()
{
	return syscall(__NR_gettid);
}
