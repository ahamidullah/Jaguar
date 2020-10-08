// One thread per core.
// Each thread is locked to a CPU core.
// Fiber pool -- max fiber count (160?).
// Priority job queues (low/normal/high).
// Each worker thread switches to a fiber, grabs a job, and starts working.
// Special I/O thread for blocking functions.
// Thread local storage? Fiber local storage? Fiber-safe thread local storage?
// Adaptive mutexes?
// Per-thread memory heap. 2mb?

#include "Job.h"
#include "Basic/Thread.h"
#include "Basic/Atomic.h"
#include "Basic/Array.h"
#include "Basic/CPU.h"
#include "Basic/Memory.h"
#include "Basic/Stack.h"
#include "Basic/Pool.h"

struct WorkerThreadParameter
{
};

struct WorkerThread
{
	Thread platformThread;
	WorkerThreadParameter parameter;
};

void *WorkerThreadProcedure(void *);
void JobFiberProcedure(void *);

// @TODO: Arena allocator.
// @TODO: Fixed vs static?
// @TODO: More granular locking?
auto jobLock = Spinlock{};
auto workerThreads = []() -> Array<WorkerThread>
{
	auto wts = NewArrayIn<WorkerThread>(GlobalAllocator(), WorkerThreadCount());
	wts.Last()->platformThread = CurrentThread();
	for (auto i = 0; i < wts.count - 1; i += 1)
	{
		wts[i].platformThread = NewThread(WorkerThreadProcedure, &wts[i].parameter);
	}
	for (auto i = 0; i < wts.count; i++)
	{
		SetThreadProcessorAffinity(wts[i].platformThread, i);
	}
	return wts;
}();
auto idleJobFibers = []() -> FixedPool<JobFiber, JobFiberCount>
{
	auto p = NewFixedPool<JobFiber, JobFiberCount>();
	for (auto &f : p)
	{
		f.platformFiber = NewFiber(JobFiberProcedure, &f.parameter);
	}
	return p;
}();
auto jobQueues = MakeStaticArrayInit<Stack<Job>, JobPriorityCount>(NewStackIn<Job>(GlobalAllocator(), 64));
auto waitingJobFiberQueue = NewStackIn<JobFiber *>(GlobalAllocator(), 64);
auto resumableJobFiberQueues = MakeStaticArrayInit<Array<JobFiber *>, JobPriorityCount>(NewArrayIn<JobFiber *>(GlobalAllocator(), 0));
auto jobCounters = NewSlotAllocator(sizeof(JobCounter), alignof(JobCounter), JobFiberCount, JobFiberCount, GlobalAllocator(), GlobalAllocator());
ThreadLocal auto runningJobFiber = (JobFiber *){};
ThreadLocal auto workerThreadFiber = Fiber{};

void JobFiberProcedure(void *param)
{
	auto p = (JobFiberParameter *)param;
	while (true)
	{
		p->runningJob.procedure(p->runningJob.parameter);
		p->runningJob.finished = true;
		workerThreadFiber.Switch();
	}
}

void *WorkerThreadProcedure(void *)
{
	ConvertThreadToFiber(&workerThreadFiber);
	while (true)
	{
		auto runFiber = (JobFiber *){};
		jobLock.Lock();
		for (auto i = (s64)HighPriorityJob; i <= LowPriorityJob; i += 1)
		{
			if (resumableJobFiberQueues[i].count > 0)
			{
				runFiber = resumableJobFiberQueues[i].PopLast();
				break;
			}
			else if (jobQueues[i].Count() > 0)
			{
				if (idleJobFibers.Available() == 0)
				{
					break;
				}
				runFiber = idleJobFibers.Get();
				runFiber->parameter.runningJob = jobQueues[i].Pop();
				break;
			}
		}
		jobLock.Unlock();
		if (!runFiber)
		{
			continue;
		}
		runningJobFiber = runFiber;
		runFiber->platformFiber.Switch();
		auto runJob = &runFiber->parameter.runningJob;
		jobLock.Lock();
		Defer(jobLock.Unlock());
		if (runJob->finished)
		{
			idleJobFibers.Release(runFiber);
			if (!runJob->waitingCounter)
			{
				continue;
			}
			runJob->waitingCounter->unfinishedJobCount -= 1;
			if (runJob->waitingCounter->unfinishedJobCount == 0)
			{
				for (auto &f : runJob->waitingCounter->waitingFibers)
				{
					resumableJobFiberQueues[f->parameter.runningJob.priority].Append(f);
				}
			}
		}
	}
}

void InitializeJobs(JobProcedure initProc, void *initParam)
{
	auto j = NewJobDeclaration(initProc, initParam);
	RunJobs(NewArrayView(&j, 1), HighPriorityJob, NULL);
	WorkerThreadProcedure(&workerThreads.Last()->parameter);
}

JobDeclaration NewJobDeclaration(JobProcedure proc, void *param)
{
	return
	{
		.procedure = proc,
		.parameter = param,
	};
}

void RunJobs(ArrayView<JobDeclaration> js, JobPriority p, JobCounter **c)
{
	jobLock.Lock();
	Defer(jobLock.Unlock());
	auto counter = (JobCounter *){};
	if (c)
	{
		*c = (JobCounter *)jobCounters.Allocate(sizeof(JobCounter));
		(*c)->jobCount = js.count;
		(*c)->unfinishedJobCount = js.count;
		Assert((*c)->waitingFibers.capacity == 0 && (*c)->waitingFibers.count == 0);
		counter = *c;
	}
	for (auto j : js)
	{
		jobQueues[p].Push(
		{
			.priority = p,
			.procedure = j.procedure,
			.parameter = j.parameter,
			.waitingCounter = counter,
		});
	}
}

void JobCounter::Wait()
{
	// @TODO: Have a per-counter lock?
	jobLock.Lock();
	if (this->unfinishedJobCount == 0)
	{
		return;
	}
	this->waitingFibers.Append(runningJobFiber);
	jobLock.Unlock();
	workerThreadFiber.Switch();
}

void JobCounter::Reset()
{
	jobLock.Lock();
	Defer(jobLock.Unlock());
	this->unfinishedJobCount = this->jobCount;
	this->waitingFibers.Resize(0);
}

void JobCounter::Free()
{
	jobLock.Lock();
	Defer(jobLock.Unlock());
	jobCounters.Deallocate(this);
}

s64 WorkerThreadCount()
{
	//return CPUProcessorCount();
	return 4;
}
