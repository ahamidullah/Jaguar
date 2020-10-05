#include "../Thread.h"
#include "../Log.h"
#include "../Process.h"
#include "../String.h"
#include "../Memory.h"

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

void Spinlock::Lock()
{
	while (true)
	{
		if (AtomicCompareAndSwap64(&this->handle, 0, 1) == 0)
		{
			return;
		}
		while (this->handle != 0)
		{
			CPUHintSpinWaitLoop();
		}
	}
}

void Spinlock::Unlock()
{
	Assert(this->handle == 1);
	this->handle = 0;
}

bool Spinlock::IsLocked()
{
	return this->handle == 1;
}

ThreadLocal auto threadIndex = 0;
auto threadCount = s64{1};

struct ThreadStartParameters
{
	s64 threadIndex;
	ThreadProcedure procedure;
	void *param;
};

void *ThreadStart(void *param)
{
	auto p = (ThreadStartParameters *)param;
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
	auto i = AtomicFetchAndAdd64(&threadCount, 1);
	auto p = (ThreadStartParameters *)GlobalAllocator()->Allocate(sizeof(ThreadStartParameters));
	p->threadIndex = i;
	p->procedure = proc;
	p->param = param;
	auto t = Thread{};
	if (pthread_create(&t, &attrs, ThreadStart, p))
	{
		Abort("Thread", "Failed pthread_create(): %k.", PlatformError());
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
		Abort("Thread", "Failed pthread_setaffinity_np(): %k.", PlatformError());
	}
}

const auto MaxThreadNameLength = 15;

void SetThreadName(String n)
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

String ThreadName()
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

s64 ThreadID()
{
	return syscall(__NR_gettid);
}

s64 ThreadIndex()
{
	return threadIndex;
}

s64 ThreadCount()
{
	return threadCount;
}
