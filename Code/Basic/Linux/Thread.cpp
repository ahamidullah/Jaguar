#include "../Thread.h"
#include "../Log.h"
#include "../Process.h"
#include "../String.h"
#include "../Memory.h"

ThreadLocal auto threadIndex = 0;
auto threadCount = s64{1};

struct ThreadProcedureWrapperParameters
{
	s64 threadIndex;
	ThreadProcedure procedure;
	void *param;
};

void *ThreadProcedureWrapper(void *param)
{
	auto p = (ThreadProcedureWrapperParameters *)param;
	threadIndex = p->threadIndex;
	return p->procedure(p->param);
}

Thread NewThread(ThreadProcedure proc, void *param)
{
	auto attrs = pthread_attr_t{};
	if (pthread_attr_init(&attrs))
	{
		Abort("Thread", "Failed on pthread_attr_init(): %k.", PlatformError());
	}
	auto i = AtomicAdd64(&threadCount, 1);
	auto wrapperParam = (ThreadProcedureWrapperParameters *)GlobalHeap()->Allocate(sizeof(ThreadProcedureWrapperParameters));
	wrapperParam->threadIndex = i;
	wrapperParam->procedure = proc;
	wrapperParam->param = param;
	auto t = Thread{};
	if (pthread_create(&t, &attrs, ThreadProcedureWrapper, wrapperParam))
	{
		Abort("Thread", "Failed on pthread_create(): %k.", PlatformError());
	}
	return t;
}

void SetThreadProcessorAffinity(Thread t, s64 cpuIndex)
{
	auto cs = cpu_set_t{};
	CPU_ZERO(&cs);
	CPU_SET(cpuIndex, &cs);
	if (pthread_setaffinity_np(t, sizeof(cs), &cs))
	{
		Abort("Thread", "Failed on pthread_setaffinity_np(): %k.", PlatformError());
	}
}

const auto MaxThreadNameLength = 15;

void SetCurrentThreadName(String n)
{
	if (n.Length() > MaxThreadNameLength)
	{
		LogError("Thread", "Failed to set name for thread %k: name is too long (max: %d).\n", n, MaxThreadNameLength);
		return;
	}
	if (prctl(PR_SET_NAME, &n[0], 0, 0, 0) != 0)
	{
		LogError("Thread", "Failed to set name for thread %k: %k.\n", n, PlatformError());
	}
}

String CurrentThreadName()
{
	auto buf = (char *)AllocateMemory(MaxThreadNameLength);
	if (prctl(PR_GET_NAME, buf, 0, 0, 0) != 0)
	{
		LogError("Thread", "Failed to get thread name: %k.\n", PlatformError());
	}
	return String{buf};
}

Thread CurrentThread()
{
	return pthread_self();
}

s64 CurrentThreadID()
{
	return syscall(__NR_gettid);
}

s64 CurrentThreadIndex()
{
	return threadIndex;
}

s64 ThreadCount()
{
	return threadCount;
}
