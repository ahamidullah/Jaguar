#pragma once

#include "../String.h"
#include "../PCH.h"

#include "Code/Common.h"

#define THREAD_LOCAL __thread

typedef pthread_t Thread;
typedef void *(*ThreadProcedure)(void *);

Thread NewThread(ThreadProcedure proc, void *param);
void SetThreadName(Thread t, String n);
String ThreadName(Thread t);
Thread CurrentThread();
void SetThreadProcessorAffinity(Thread t, s64 cpuIndex);
s64 ThreadID();
