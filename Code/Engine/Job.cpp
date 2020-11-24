// Special I/O thread for blocking functions.
// Thread local storage? Fiber local storage? Fiber-safe thread local storage?
// Adaptive mutexes?
// Per-thread memory heap. 2mb?

#include "Job.h"
#include "Basic/Thread.h"
#include "Basic/Atomic.h"
#include "Basic/Container/Array.h"
#include "Basic/Container/Dequeue.h"
#include "Basic/CPU.h"
#include "Basic/Pool.h"
#include "Basic/Memory/GlobalHeap.h"

struct WorkerThreadParameter
{
	s64 threadIndex;
};

struct WorkerThread
{
	Thread platformThread;
	WorkerThreadParameter parameter;
};

void *WorkerThreadProcedure(void *);
void JobFiberProcedure(void *);

// @TODO: More granular locking?
auto jobLock = Spinlock{};
auto workerThreads = array::Array<WorkerThread>{};
auto idleJobFiberPool = []() -> pool::Static<JobFiber, JobFiberCount>
{
	auto p = pool::NewStatic<JobFiber, JobFiberCount>();
	for (auto &f : p)
	{
		f.platformFiber = NewFiber(JobFiberProcedure, &f.parameter);
	}
	return p;
}();
auto jobQueues = []() -> array::Static<dequeue::Dequeue<QueuedJob>, JobPriorityCount>
{
	auto a = array::Static<dequeue::Dequeue<QueuedJob>, JobPriorityCount>{};
	for (auto &s : a)
	{
		s = dequeue::NewWithBlockSizeIn<QueuedJob>(Memory::GlobalHeap(), 1024, 1024);
	}
	return a;
}();
auto resumableJobFiberQueues = []() -> array::Static<array::Array<JobFiber *>, JobPriorityCount>
{
	auto a = array::Static<array::Array<JobFiber *>, JobPriorityCount>{};
	for (auto &q : a)
	{
		q = array::NewWithCapacityIn<JobFiber *>(Memory::GlobalHeap(), 100);
	}
	return a;
}();
auto jobCounterPool = pool::NewIn<JobCounter>(Memory::GlobalHeap(), 0);
auto runningJobFibers = array::New<JobFiber *>(WorkerThreadCount());
//ThreadLocal auto runningJobFiber = (JobFiber *){};
//ThreadLocal auto workerThreadFiber = Fiber{};
auto wtf = array::New<bool>(WorkerThreadCount());
auto workerThreadFibers = array::New<Fiber>(WorkerThreadCount());
ThreadLocal auto waitingJobCounter = (JobCounter *){};

#include "string.h"
#include <stdio.h>

void JobFiberProcedure(void *param)
{
	auto p = (JobFiberParameter *)param;
	while (true)
	{
		p->procedure(p->parameter);
		p->finished = true;
		auto ii = p->threadIndex;
		//printf("Switch to worker: %ld\n", ii + 1);
		//printf("ThreadID: %ld, JobFiberProcedure, finished job %p, switching back to worker thread %ld\n", ThreadID(), p->procedure, ii);
		workerThreadFibers[ThreadIndex()].Switch();
	}
}

void RunGame(void *);
void Test2(void *);

void *WorkerThreadProcedure(void *param)
{
	//auto ii = ThreadIndex();
	auto p = (WorkerThreadParameter *)param;
	//printf("ThreadID: %ld, ThreadIndex: %ld\n", ThreadID(), p->threadIndex);
	//printf("RunGame: %p, Test2: %p\n", RunGame, Test2);
	auto ii = p->threadIndex;
	ConvertThreadToFiber(&workerThreadFibers[ThreadIndex()]);
	auto ff = (JobFiber *){};
	auto fff = JobFiber{};
	while (true)
	{
		auto runFiber = (JobFiber *){};
		jobLock.Lock();
		for (auto i = (s64)HighJobPriority; i <= LowJobPriority; i += 1)
		{
			if (resumableJobFiberQueues[i].count > 0)
			{
				runFiber = resumableJobFiberQueues[i].Pop();
				//printf("resuming\n");
				//Assert(runFiber->parameter.procedure == RunGame);
				break;
			}
			else if (jobQueues[i].count > 0)
			{
				if (idleJobFiberPool.Available() == 0)
				{
					break;
				}
				runFiber = idleJobFiberPool.Get();
				auto j = jobQueues[i].PopBack();
				runFiber->parameter = JobFiberParameter
				{
					.priority = (JobPriority)i,
					.procedure = j.procedure,
					.parameter = j.parameter,
					.waitingCounter = j.waitingCounter,
					.threadIndex = ii,
				};
				if (j.waitingCounter)
				{
					//Assert(j.waitingCounter->waitingFibers.count == 1);
					//Assert(runFiber->parameter.waitingCounter->waitingFibers.count == 1);
				}
				//Assert(runFiber->parameter.procedure);
				break;
			}
		}
		ff = runFiber;
		if (ff)
		{
			//printf("GET %p %ld\n", runFiber, ii + 1);
			fff = *runFiber;
		}
		jobLock.Unlock();
		if (!runFiber)
		{
			continue;
		}
		runningJobFibers[ThreadIndex()] = runFiber;
		//printf("ThreadID: %ld, WorkerThreadProcedure, running job fiber %p(%p)\n", ThreadID(), runFiber, runFiber->parameter.procedure);
		runFiber->platformFiber.Switch();
		#if 0
		jobLock.Lock();
		Assert(ff == runFiber);
		memcmp(runFiber, &ff, sizeof(JobFiber));
		//printf("RELEASE %p, %ld\n", runFiber, ii + 1);
		if (runFiber->parameter.finished)
		{
			idleJobFiberPool.Release(runFiber);
			//ConsolePrint("%ld\n", idleJobFiberPool.Available());
		}
		jobLock.Unlock();
		#else
		jobLock.Lock();
		Defer(jobLock.Unlock());
		//Assert(runFiber->parameter.waitingCounter->waitingFibers.count == 1);
		if (runFiber->parameter.finished)
		{
			//printf("ThreadID: %ld, WorkerThreadProcedure, done with fiber %p(%p), finished job, finished counter %p\n", ThreadID(), runFiber, runFiber->parameter.procedure, runFiber->parameter.waitingCounter);
			idleJobFiberPool.Release(runFiber);
			if (!runFiber->parameter.waitingCounter)
			{
				continue;
			}
			runFiber->parameter.waitingCounter->unfinishedJobCount -= 1;
			//Assert(runFiber->parameter.waitingCounter->unfinishedJobCount == 0);
			//Assert(runFiber->parameter.waitingCounter->waitingFibers.count == 1);
			if (runFiber->parameter.waitingCounter->unfinishedJobCount == 0)
			{
				for (auto &f : runFiber->parameter.waitingCounter->waitingFibers)
				{
					resumableJobFiberQueues[f->parameter.priority].Append(f);
				}
			}
		//	printf("rq: %ld\n", resumableJobFiberQueues[0].count);
		}
		else
		{
			//printf("ThreadID: %ld, WorkerThreadProcedure, done with fiber, retiring fiber %p(%p)\n", ThreadID(), runFiber, runFiber->parameter.procedure);
			// @TODO: Job counter locking.
			if (waitingJobCounter->unfinishedJobCount == 0)
			{
				// All of the dependency jobs already finished. This job can resume immediately.
				resumableJobFiberQueues[runFiber->parameter.priority].Append(runFiber);
			}
			else
			{
				// The dependency jobs are still running. Add this fiber to the wait list.
				waitingJobCounter->waitingFibers.Append(runFiber);
			}
		}
		#endif
	}
}

void InitializeJobs(JobProcedure initProc, void *initParam)
{
	workerThreads = array::NewIn<WorkerThread>(Memory::GlobalHeap(), WorkerThreadCount());
	workerThreads[0].platformThread = CurrentThread();
	//workerThreads.Last()->platformThread = CurrentThread();
	for (auto i = 1; i < workerThreads.count; i += 1)
	//for (auto i = 0; i < workerThreads.count - 1; i += 1)
	{
		workerThreads[i].parameter.threadIndex = i;
		workerThreads[i].platformThread = NewThread(WorkerThreadProcedure, &workerThreads[i].parameter);
	}
	for (auto i = 0; i < workerThreads.count; i += 1)
	{
		wtf[i] = true;
		SetThreadProcessorAffinity(workerThreads[i].platformThread, i);
	}
	auto j = NewJobDeclaration(initProc, initParam);
	RunJobs(array::NewView(&j, 1), HighJobPriority, NULL);
	WorkerThreadProcedure(&workerThreads[0].parameter);
	//WorkerThreadProcedure(&workerThreads.Last()->parameter);
}

JobDeclaration NewJobDeclaration(JobProcedure proc, void *param)
{
	return
	{
		.procedure = proc,
		.parameter = param,
	};
}

void RunJobs(array::View<JobDeclaration> js, JobPriority p, JobCounter **c)
{
	jobLock.Lock();
	Defer(jobLock.Unlock());
	auto counter = (JobCounter *){};
	if (c)
	{
		counter = jobCounterPool.Get();
		counter->jobCount = js.count;
		counter->unfinishedJobCount = js.count;
		counter->waitingFibers.SetAllocator(Memory::GlobalHeap());
		Assert(counter->waitingFibers.capacity == 0 && counter->waitingFibers.count == 0);
		*c = counter;
	/*
		*c = jobCounterPool.Get();
		(*c)->jobCount = js.count;
		(*c)->unfinishedJobCount = js.count;
		(*c)->waitingFibers.SetAllocator(GlobalAllocator());
		Assert((*c)->waitingFibers.capacity == 0 && (*c)->waitingFibers.count == 0);
		counter = *c;
	*/
	}
	for (auto j : js)
	{
		Assert(j.procedure);
		auto qj = QueuedJob
		{
			.procedure = j.procedure,
			.parameter = j.parameter,
			.waitingCounter = counter,
		};
		if (c)
		{
			//Assert(qj.waitingCounter->waitingFibers.count == 0);
		}
		jobQueues[p].PushFront(qj);
		/*
		jobQueues[p].PushFront(
		{
			.procedure = j.procedure,
			.parameter = j.parameter,
			.waitingCounter = counter,
		});
		*/
	}
	if (c)
	{
		//Assert(counter->jobCount == 1);
		//Assert(counter->unfinishedJobCount == 1);
		//Assert(counter->waitingFibers.count == 0);
	}
}

void JobCounter::Wait()
{
	if (this->unfinishedJobCount == 0)
	{
		// The jobs already finished.
		//printf("ThreadID: %ld, JobCounter::Wait(), job counter %p already finished, returning\n", ThreadID(), this);
		return;
	}
	waitingJobCounter = this;
	auto ii = ThreadIndex();
	#if 0
	// @TODO: Have a per-counter lock?
	{
		jobLock.Lock();
		Defer(jobLock.Unlock());
		if (this->unfinishedJobCount == 0)
		{
			//Assert(0);
			printf("ThreadID: %ld, JobCounter::Wait(), job counter %p already finished, returning\n", ThreadID(), this);
			return;
		}
		this->waitingFibers.Append(runningJobFibers[ii]);
		Assert(wtf[ii] == false);
	}
	#endif
	//Assert(this->waitingFibers.count == 1);
	//printf("ThreadID: %ld, JobCounter::Wait(), waiting on job counter %p, switching to worker thread %ld\n", ThreadID(), this, ii);
	workerThreadFibers[ThreadIndex()].Switch();
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
	jobCounterPool.Release(this);
}

s64 WorkerThreadCount()
{
	return CPUProcessorCount();
}
