#pragma once

#include "../String.h"
#include "../PCH.h"

#include "Common.h"

#define ThreadLocal __thread

typedef pthread_t Thread;
typedef void *(*ThreadProcedure)(void *);

Thread NewThread(ThreadProcedure proc, void *param);
void SetThreadProcessorAffinity(Thread t, s64 cpuIndex);
void SetThreadName(Thread t, String n);
String ThreadName(Thread t);
Thread CurrentThread();
s64 CurrentThreadID();
