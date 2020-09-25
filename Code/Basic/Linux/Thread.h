#pragma once

#include "../String.h"
#include "../PCH.h"
#include "Common.h"

struct Mutex
{
	pthread_mutex_t handle;

	void Lock();
	void Unlock();
};

Mutex NewMutex();

struct Semaphore
{
	sem_t handle;

	void Signal();
	void Wait();
	s64 Value();
};

Semaphore NewSemaphore(s64 val);

struct Spinlock
{
	volatile s64 handle;

	void Lock();
	void Unlock();
	bool IsLocked();
};

#define ThreadLocal __thread

typedef pthread_t Thread;
typedef void *(*ThreadProcedure)(void *);

Thread NewThread(ThreadProcedure proc, void *param);
void SetThreadProcessorAffinity(Thread t, s64 cpuIndex);
void SetThreadName(Thread t, String n);
String ThreadName(Thread t);
Thread CurrentThread();
s64 ThreadID();
s64 ThreadIndex();
s64 ThreadCount();
