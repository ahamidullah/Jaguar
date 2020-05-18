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

constexpr auto MAX_JOBS_PER_QUEUE = 100;

#define JOB_FIBER_POINTER_SENTINEL ((JobFiber *)0xDEADBEEF)

struct Job
{
	JobProcedure procedure;
	void *parameter;
	JobPriority priority;
	JobCounter *waitingCounter;
	bool finished;
};

struct JobFiberParameter
{
	Job scheduledJob; // @TODO: Running job?
};

struct JobFiber
{
	Fiber platformFiber;
	JobFiberParameter parameter;
	struct JobFiber *next;
};

struct WorkerThreadParameter
{
	u32 threadIndex;
};

struct WorkerThread
{
	ThreadHandle platformThread;
	WorkerThreadParameter parameter;
};

static Array<WorkerThread> workerThreads;
static AtomicLinkedList<JobFiber> idleJobFiberList;
static AtomicRingBuffer<Job, MAX_JOBS_PER_QUEUE> jobQueues[JOB_PRIORITY_COUNT];
static AtomicRingBuffer<JobFiber *, MAX_JOBS_PER_QUEUE> resumableJobQueues[JOB_PRIORITY_COUNT];
static Semaphore jobsAvailableSemaphore;

static THREAD_LOCAL Fiber workerThreadFiber;
static THREAD_LOCAL JobCounter *waitingJobCounter;

static THREAD_LOCAL u32 threadIndex;

u32 GetThreadIndex()
{
	return threadIndex;
}

void *WorkerThreadProcedure(void *parameter)
{
	ConvertThreadToFiber(&workerThreadFiber);
	threadIndex = ((WorkerThreadParameter *)parameter)->threadIndex;

	JobFiber *activeJobFiber = NULL;
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

			SwitchToFiber(&activeJobFiber->platformFiber);

			auto scheduledJob = &activeJobFiber->parameter.scheduledJob;
			if (scheduledJob->finished)
			{
				PushToFrontOfAtomicLinkedList(&idleJobFiberList, activeJobFiber);
				if (!scheduledJob->waitingCounter)
				{
					continue;
				}
				// Check if the completion of this job caused the associated job counter to reach zero. If so, mark the job waiting on the counter as resumable.
				auto unfinishedJobCount = AtomicAdd(&scheduledJob->waitingCounter->unfinishedJobCount, -1);
				if (unfinishedJobCount > 0)
				{
					continue;
				}
				// Check to see if the parent job started waiting on the counter yet. If it hasn't then the job counter's waitingJobFiber will be NULL.
				auto waitingJobFiber = (JobFiber *)AtomicFetchAndSet((void *volatile *)&scheduledJob->waitingCounter->waitingJobFiber, JOB_FIBER_POINTER_SENTINEL);
				if (!waitingJobFiber)
				{
					continue;
				}
				// OK, the job finished, the counter reached zero, and the parent job started waiting on the counter. Now we can mark the parent job as resumable.
				WriteToAtomicRingBuffer(&resumableJobQueues[waitingJobFiber->parameter.scheduledJob.priority], waitingJobFiber);
			}
			else
			{
				if (AtomicCompareAndSwap((void *volatile *)&waitingJobCounter->waitingJobFiber, NULL, activeJobFiber) == JOB_FIBER_POINTER_SENTINEL)
				{
					// The waitingJobFiber was not NULL, hence all dependency jobs already finished. This job can resume immediately.
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
		WriteToAtomicRingBuffer(&jobQueues[priority],
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
	waitingJobCounter = counter;
	SwitchToFiber(&workerThreadFiber);
}

void InitializeJobs(JobProcedure initialJobProcedure, void *initialJobParameter)
{
	for (auto i = 0; i < JOB_FIBER_COUNT; i++)
	{
		JobFiber *newFiber = AllocateStructMemory(JobFiber);
		CreateFiber(&newFiber->platformFiber, JobFiberProcedure, &newFiber->parameter);
		newFiber->next = idleJobFiberList.head;
		idleJobFiberList.head = newFiber;
	}
	jobsAvailableSemaphore = CreateSemaphore(0);

	auto workerThreadCount = GetProcessorCount();
	workerThreads = CreateArray<WorkerThread>(workerThreadCount);
	for (auto i = 0; i < workerThreadCount - 1; i++)
	{
		workerThreads[i].parameter.threadIndex = i;
		workerThreads[i].platformThread = CreateThread(WorkerThreadProcedure, &workerThreads[i].parameter);
	}
	workerThreads[workerThreadCount - 1].parameter.threadIndex = workerThreadCount - 1;
	workerThreads[workerThreadCount - 1].platformThread = GetCurrentThread();

	for (auto i = 0; i < workerThreadCount; i++)
	{
		SetThreadProcessorAffinity(workerThreads[i].platformThread, i);
	}

	JobDeclaration initialJob = CreateJob(initialJobProcedure, initialJobParameter);
	RunJobs(1, &initialJob, HIGH_PRIORITY_JOB, NULL);
	WorkerThreadProcedure(&workerThreads[workerThreadCount - 1].parameter);
}

s64 GetWorkerThreadCount()
{
	return ArrayLength(workerThreads);
}
