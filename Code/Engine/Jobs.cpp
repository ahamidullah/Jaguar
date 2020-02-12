// One thread per core.
// Each thread is locked to a CPU core.
// Fiber pool -- max fiber count (160?).
// Priority job queues (low/normal/high).
// Each worker thread switches to a fiber, grabs a job, and starts working.
// Special I/O thread for blocking functions.
// All job synchronization is done through atomic counters (WaitForCounter).
// Thread local storage? Fiber local storage? Fiber-safe thread local storage?
// Adaptive mutexes?
// Feed-forward, game logic and rendering done simultaneously.
// Per-frame state. Delta time, frame number, skinning matrices, etc.
// Per-thread memory heap. 2mb?

// @TODO: If the parent job does not wait on the counter, and the counter gets deallocated, then the job may access the freed counter memory.
//        Need a better way to handle jobs not waiting on the counter. Maybe the jobs system should own the counter memory?
// @TODO: Check to see what should be volatile.
// @TODO: For now, you have to eventually wait for a counter or else you get a zombie resumable job.
//        We could create a function to mark the job as not rejoining.
// @TODO: We could create some mechanism to detect zombie jobs.
// @TODO: Should we post to the job semaphore when we schedule a resumable job? I can't think of a situation where it would actually help but it seems like we should...
// @TODO: Move the jobs context into the game state (jobs_state?).

#include "AtomicRingBuffer.h"
#include "AtomicLinkedList.h"
#include "Jobs.h"

constexpr auto MAX_JOBS_PER_QUEUE = 100;

#define JOB_FIBER_POINTER_SENTINEL ((JobFiber *)0xDEADBEEF)

struct Job
{
	JobProcedure procedure;
	void *parameter;
	JobPriority priority;
	JobCounter *counterWaitingForThisJob;
	bool finished;
};

struct JobFiberParameter
{
	Job scheduledJob; // @TODO: Running job?
};

struct JobFiber
{
	PlatformFiber platformFiber;
	JobFiberParameter parameter;
	struct JobFiber *next;
};

struct WorkerThreadParameter
{
	u32 threadIndex;
};

struct WorkerThread
{
	PlatformThreadHandle platformThread;
	WorkerThreadParameter parameter;
};

struct
{
	Array<WorkerThread> workerThreads;
	AtomicLinkedList<JobFiber> idleJobFiberList;
	AtomicRingBuffer<Job, MAX_JOBS_PER_QUEUE> jobQueues[JOB_PRIORITY_COUNT];
	AtomicRingBuffer<JobFiber *, MAX_JOBS_PER_QUEUE> resumableJobQueues[JOB_PRIORITY_COUNT];
	PlatformSemaphore jobsAvailableSemaphore;
} jobsContext;

__thread struct
{
	PlatformFiber workerThreadFiber;
	//Job_Fiber *activeJobFiber;
	JobCounter *waitingJobCounter;
} threadLocalJobsContext;

void *WorkerThreadProcedure(void *parameter)
{
	PlatformConvertThreadToFiber(&threadLocalJobsContext.workerThreadFiber);
	threadIndex = ((WorkerThreadParameter *)parameter)->threadIndex;

	JobFiber *activeJobFiber = NULL;
	while (1)
	{
		PlatformWaitOnSemaphore(&jobsContext.jobsAvailableSemaphore);
		do
		{
			activeJobFiber = NULL;
			// Get the next job to run. Prefer higher priority jobs and resumable jobs.
			for (s32 i = 0; i < JOB_PRIORITY_COUNT; i++)
			{
				if (Read(&jobsContext.resumableJobQueues[i], &activeJobFiber))
				{
					break;
				}
				// Avoid copying the Job twice by getting the idle job fiber before we know if we have a job.
				// If there are no more jobs left, just write the job fiber back to the idle job fiber list.
				activeJobFiber = PopFromFront(&jobsContext.idleJobFiberList);
				if (Read(&jobsContext.jobQueues[i], &activeJobFiber->parameter.scheduledJob))
				{
					break;
				}
				else
				{
					PushToFront(&jobsContext.idleJobFiberList, activeJobFiber);
					activeJobFiber = NULL;
				}
			}
			if (!activeJobFiber)
			{
				break;
			}
			PlatformSwitchToFiber(&activeJobFiber->platformFiber);
			Job *scheduledJob = &activeJobFiber->parameter.scheduledJob;
			if (scheduledJob->finished)
			{
				PushToFront(&jobsContext.idleJobFiberList, activeJobFiber);
				if (!scheduledJob->counterWaitingForThisJob)
				{
					continue;
				}
				// Check if the completion of this job caused the associated job counter to reach zero. If so, mark the job waiting on the counter as resumable.
				s32 unfinishedJobCount = PlatformAtomicAddS32(&scheduledJob->counterWaitingForThisJob->unfinishedJobCount, -1);
				if (unfinishedJobCount > 0)
				{
					continue;
				}
				// Check to see if the parent job started waiting on the counter yet. If it hasn't then the job counter's waitingJobFiber will be NULL.
				JobFiber *waitingJobFiber = (JobFiber *)PlatformFetchAndSetPointer((void *volatile *)&scheduledJob->counterWaitingForThisJob->waitingJobFiber, JOB_FIBER_POINTER_SENTINEL);
				if (!waitingJobFiber)
				{
					continue;
				}
				// OK, the job finished, the counter reached zero, and the parent job started waiting on the counter. Now we can mark the parent job as resumable.
				Write(&jobsContext.resumableJobQueues[waitingJobFiber->parameter.scheduledJob.priority], waitingJobFiber);
			}
			else
			{
				if (PlatformCompareAndSwapPointers((void *volatile *)&threadLocalJobsContext.waitingJobCounter->waitingJobFiber, NULL, activeJobFiber) == JOB_FIBER_POINTER_SENTINEL)
				{
					// The waitingJobFiber was not NULL, hence all dependency jobs already finished. This job can resume immediately.
					Write(&jobsContext.resumableJobQueues[scheduledJob->priority], activeJobFiber);
				}
			}
		} while (activeJobFiber);
	}
	return NULL;
}

void JobFiberProcedure(void *parameterPointer)
{
	JobFiberParameter *parameter = (JobFiberParameter *)parameterPointer;
	Job *activeJob;
	while (1)
	{
		activeJob = &parameter->scheduledJob;
		activeJob->procedure(activeJob->parameter);
		activeJob->finished = 1;
		PlatformSwitchToFiber(&threadLocalJobsContext.workerThreadFiber);
	}
}

JobDeclaration CreateJob(JobProcedure procedure, void *parameter)
{
	return {
		.procedure = procedure,
		.parameter = parameter,
	};
}

void RunJobs(u32 jobCount, JobDeclaration *jobDeclarations, JobPriority priority, JobCounter *counter)
{
	if (counter)
	{
		counter->unfinishedJobCount = jobCount;
		counter->waitingJobFiber = NULL;
	}
	for (u32 i = 0; i < jobCount; i++)
	{
		Write(&jobsContext.jobQueues[priority], (Job){
			.procedure = jobDeclarations[i].procedure,
			.parameter = jobDeclarations[i].parameter,
			.priority = priority,
			.counterWaitingForThisJob = counter,
			.finished = 0,
		});
		PlatformSignalSemaphore(&jobsContext.jobsAvailableSemaphore);
	}
}

void ClearJobCounter(JobCounter *counter)
{
	counter->unfinishedJobCount = 0;
	counter->waitingJobFiber = NULL;
}

void WaitForJobCounter(JobCounter *counter)
{
	if (counter->unfinishedJobCount == 0)
	{
		return;
	}
	threadLocalJobsContext.waitingJobCounter = counter;
	PlatformSwitchToFiber(&threadLocalJobsContext.workerThreadFiber);
}

void InitializeJobs(JobProcedure initialJobProcedure, void *initialJobParameter)
{
	for (u32 i = 0; i < JOB_FIBER_COUNT; i++)
	{
		JobFiber *newFiber = AllocateStructMemory(JobFiber);
		PlatformCreateFiber(&newFiber->platformFiber, JobFiberProcedure, &newFiber->parameter);
		newFiber->next = jobsContext.idleJobFiberList.head;
		jobsContext.idleJobFiberList.head = newFiber;
	}
	jobsContext.jobsAvailableSemaphore = PlatformCreateSemaphore(0);
	//GameState->jobsContext.workerThreadCount = PlatformGetProcessorCount();

	auto workerThreadCount = PlatformGetProcessorCount();
	jobsContext.workerThreads = CreateArray<WorkerThread>(workerThreadCount);
	for (u32 i = 0; i < workerThreadCount - 1; i++)
	{
		jobsContext.workerThreads[i].parameter.threadIndex = i;
		PlatformCreateThread(WorkerThreadProcedure, &jobsContext.workerThreads[i].parameter);
	}
	jobsContext.workerThreads[workerThreadCount - 1].platformThread = PlatformGetCurrentThread();

	for (u32 i = 0; i < workerThreadCount; i++)
	{
		PlatformSetThreadProcessorAffinity(jobsContext.workerThreads[i].platformThread, i);
	}

	JobDeclaration initialJob = CreateJob(initialJobProcedure, initialJobParameter);
	RunJobs(1, &initialJob, HIGH_PRIORITY_JOB, NULL);
	WorkerThreadProcedure(&jobsContext.workerThreads[workerThreadCount - 1].parameter);
}

u32 GetWorkerThreadCount()
{
	return Length(jobsContext.workerThreads);
}
