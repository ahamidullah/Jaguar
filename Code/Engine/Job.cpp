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
	auto wts = NewArrayIn<WorkerThread>(GlobalAllocator(), CPUProcessorCount());
	for (auto &t : wts)
	{
		t.platformThread = NewThread(WorkerThreadProcedure, &t.parameter);
	}
	wts[wts.count - 1].platformThread = CurrentThread();
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
auto resumableJobFiberQueues = MakeStaticArrayInit<Array<JobFiber *>, JobPriorityCount>(Array<JobFiber *>{.allocator = GlobalAllocator()});
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
		Defer(jobLock.Unlock());
		for (auto i = (s64)HighPriorityJob; i >= 0; i -= 1)
		{
			auto empty = false;
			if (resumableJobFiberQueues[i].count > 0)
			{
				runFiber = resumableJobFiberQueues[i].PopBack();
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
		if (!runFiber)
		{
			continue;
		}
		runningJobFiber = runFiber;
		runFiber->platformFiber.Switch();
		auto runJob = &runFiber->parameter.runningJob;
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
	WorkerThreadProcedure(&workerThreads[workerThreads.count - 1].parameter);
}

JobDeclaration NewJobDeclaration(JobProcedure proc, void *param)
{
	return
	{
		.procedure = proc,
		.parameter = param,
	};
}

void RunJobs(ArrayView<JobDeclaration> jds, JobPriority p, JobCounter **c)
{
	jobLock.Lock();
	Defer(jobLock.Unlock());
	auto counter = (JobCounter *){};
	if (c)
	{
		*c = (JobCounter *)jobCounters.Allocate(sizeof(JobCounter));
		(*c)->jobCount = jds.count;
		(*c)->unfinishedJobCount = jds.count;
		Assert((*c)->waitingFibers.capacity == 0 && (*c)->waitingFibers.count == 0);
		counter = *c;
	}
	for (auto &jd : jds)
	{
		jobQueues[p].Push(
		{
			.priority = p,
			.procedure = jd.procedure,
			.parameter = jd.parameter,
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

#if 0
void *WorkerThreadProcedure(void *parameter)
{
	ConvertThreadToFiber(&workerThreadFiber);
	threadIndex = ((WorkerThreadParameter *)parameter)->threadIndex;

	auto activeJobFiber = (JobFiber *){};
	while (1)
	{
		WaitOnSemaphore(&jobsAvailableSemaphore);
		do
		{
			activeJobFiber = NULL;
			// Get the next job to run. Prefer higher priority jobs and resumable jobs.
			for (auto i = 0; i < JOB_PRIORITY_COUNT; i++)
			{
				if (ReadFromAtomicRingBuffer(&resumableJobQueues[i], &activeJobFiber))
				{
					break;
				}
				// Avoid copying the Job twice by getting the idle job fiber before we know if we have a job.
				// If there are no more jobs left, just write the job fiber back to the idle job fiber list.
				activeJobFiber = PopFromFrontOfAtomicLinkedList(&idleJobFiberList);
				if (ReadFromAtomicRingBuffer(&jobQueues[i], &activeJobFiber->parameter.scheduledJob))
				{
					break;
				}
				else
				{
					PushToFrontOfAtomicLinkedList(&idleJobFiberList, activeJobFiber);
					activeJobFiber = NULL;
				}
			}
			if (!activeJobFiber)
			{
				break;
			}

			// Calling SwitchToFiber will suspend this WorkerThreadProcedure and start running the
			// job on this thread.  The job procedure will eventually finish or call
			// WaitForJobCounter, at which point this WorkerThreadProcedure will resume.
			SwitchToFiber(&activeJobFiber->platformFiber);

			auto scheduledJob = &activeJobFiber->parameter.scheduledJob;
			if (scheduledJob->finished)
			{
				PushToFrontOfAtomicLinkedList(&idleJobFiberList, activeJobFiber);
				if (!scheduledJob->waitingCounter)
				{
					continue;
				}
				// Check if the completion of this job caused the associated job counter to reach
				// zero. If so, mark the job waiting on the counter as resumable.
				if (auto unfinishedJobCount = AtomicAdd(&scheduledJob->waitingCounter->unfinishedJobCount, -1); unfinishedJobCount > 0)
				{
					continue;
				}
				// Check to see if the parent job started waiting on the counter yet. If it hasn't
				// then the job counter's waitingJobFiber will be NULL.
				auto waitingJobFiber = (JobFiber *)AtomicFetchAndSet((void *volatile *)&scheduledJob->waitingCounter->waitingJobFiber, JOB_FIBER_POINTER_SENTINEL);
				if (!waitingJobFiber)
				{
					continue;
				}
				// OK, the job finished, the counter reached zero, and the parent job started
				// waiting on the counter. Now we can mark the parent job as resumable.
				WriteToAtomicRingBuffer(&resumableJobQueues[waitingJobFiber->parameter.scheduledJob.priority], waitingJobFiber);
			}
			else
			{
				if (AtomicCompareAndSwap((void *volatile *)&waitingJobCounter->waitingJobFiber, NULL, activeJobFiber) == JOB_FIBER_POINTER_SENTINEL)
				{
					// The waitingJobFiber was not NULL, hence all dependency jobs already finished.
					// This job can resume immediately.
					WriteToAtomicRingBuffer(&resumableJobQueues[scheduledJob->priority], activeJobFiber);
				}
			}
		} while (activeJobFiber);
	}
	return NULL;
}

void JobFiberProcedure(void *parameterPointer)
{
	auto parameter = (JobFiberParameter *)parameterPointer;
	Job *activeJob;
	while (1)
	{
		activeJob = &parameter->scheduledJob;
		activeJob->procedure(activeJob->parameter);
		activeJob->finished = 1;
		SwitchToFiber(&workerThreadFiber);
	}
}

JobDeclaration CreateJob(JobProcedure procedure, void *parameter)
{
	return
	{
		.procedure = procedure,
		.parameter = parameter,
	};
}

// @TODO: Switch to a real array.
void RunJobs(u32 jobCount, JobDeclaration *jobDeclarations, JobPriority priority, JobCounter *counter)
{
	if (counter)
	{
		counter->unfinishedJobCount = jobCount,
		counter->waitingJobFiber = NULL;
	}
	for (auto i = 0; i < jobCount; i++)
	{
		WriteToAtomicRingBuffer(
			&jobQueues[priority],
			{
				.procedure      = jobDeclarations[i].procedure,
				.parameter      = jobDeclarations[i].parameter,
				.priority       = priority,
				.waitingCounter = counter,
				.finished       = 0,
			});
		SignalSemaphore(&jobsAvailableSemaphore);
	}
}

void WaitForJobCounter(JobCounter *counter)
{
	if (counter->unfinishedJobCount == 0)
	{
		return;
	}
	waitingJobCounter = counter;
	SwitchToFiber(&workerThreadFiber);
}

#endif

s64 GetWorkerThreadCount()
{
	return 4;
	//return ArrayLength(workerThreads);
}
