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

struct Job {
	JobProcedure Procedure;
	void *Parameter;
	JobPriority Priority;
	JobCounter *CounterWaitingForThisJob;
	bool Finished;
};

struct JobFiberParameter {
	Job ScheduledJob; // @TODO: Running job?
};

struct JobFiber {
	PlatformFiber platformFiber;
	JobFiberParameter parameter;
	struct JobFiber *next;
};

struct WorkerThreadParameter {
	u32 ThreadIndex;
};

struct WorkerThread {
	PlatformThreadHandle PlatformThread;
	WorkerThreadParameter Parameter;
};

struct Context {
	WorkerThread *WorkerThreads;
	AtomicLinkedList<JobFiber> IdleJobFiberList;
	AtomicRingBuffer<Job, MAX_JOBS_PER_QUEUE> JobQueues[JOB_PRIORITY_COUNT];
	AtomicRingBuffer<JobFiber *, MAX_JOBS_PER_QUEUE> ResumableJobQueues[JOB_PRIORITY_COUNT];
	PlatformSemaphore JobsAvailableSemaphore;
} jobsContext;

__thread struct {
	PlatformFiber worker_thread_fiber;
	//Job_Fiber *active_job_fiber;
	JobCounter *waiting_job_counter;
} thread_local_jobsContext;

void *WorkerThreadProcedure(void *parameter) {
	PlatformConvertThreadToFiber(&thread_local_jobsContext.worker_thread_fiber);
	thread_index = ((WorkerThreadParameter *)parameter)->ThreadIndex;
	JobFiber *ActiveJobFiber = NULL;
	while (1) {
		PlatformWaitOnSemaphore(&jobsContext.JobsAvailableSemaphore);
		do {
			ActiveJobFiber = NULL;
			// Get the next job to run. Prefer higher priority jobs and resumable jobs.
			for (s32 i = 0; i < JOB_PRIORITY_COUNT; i++) {
				if (Read(&jobsContext.ResumableJobQueues[i], &ActiveJobFiber)) {
					break;
				}
				// Avoid copying the Job twice by getting the idle job fiber before we know if we have a job.
				// If there are no more jobs left, just write the job fiber back to the idle job fiber list.
				ActiveJobFiber = PopFromFront(&jobsContext.IdleJobFiberList);
				if (Read(&jobsContext.JobQueues[i], &ActiveJobFiber->parameter.ScheduledJob)) {
					break;
				} else {
					PushToFront(&jobsContext.IdleJobFiberList, ActiveJobFiber);
					ActiveJobFiber = NULL;
				}
			}
			if (!ActiveJobFiber) {
				break;
			}
			PlatformSwitchToFiber(&ActiveJobFiber->platformFiber);
			Job *ScheduledJob = &ActiveJobFiber->parameter.ScheduledJob;
			if (ScheduledJob->Finished) {
				PushToFront(&jobsContext.IdleJobFiberList, ActiveJobFiber);
				if (!ScheduledJob->CounterWaitingForThisJob) {
					continue;
				}
				// Check if the completion of this job caused the associated job counter to reach zero. If so, mark the job waiting on the counter as resumable.
				s32 UnfinishedJobCount = PlatformAtomicAddS32(&ScheduledJob->CounterWaitingForThisJob->UnfinishedJobCount, -1);
				if (UnfinishedJobCount > 0) {
					continue;
				}
				// Check to see if the parent job started waiting on the counter yet. If it hasn't then the job counter's waitingJobFiber will be NULL.
				JobFiber *WaitingJobFiber = (JobFiber *)PlatformFetchAndSetPointer((void *volatile *)&ScheduledJob->CounterWaitingForThisJob->WaitingJobFiber, JOB_FIBER_POINTER_SENTINEL);
				if (!WaitingJobFiber) {
					continue;
				}
				// OK, the job finished, the counter reached zero, and the parent job started waiting on the counter. Now we can mark the parent job as resumable.
				Write(&jobsContext.ResumableJobQueues[WaitingJobFiber->parameter.ScheduledJob.Priority], WaitingJobFiber);
			} else {
				if (PlatformCompareAndSwapPointers((void *volatile *)&thread_local_jobsContext.waiting_job_counter->WaitingJobFiber, NULL, ActiveJobFiber) == JOB_FIBER_POINTER_SENTINEL) {
					// The waitingJobFiber was not NULL, hence all dependency jobs already finished. This job can resume immediately.
					Write(&jobsContext.ResumableJobQueues[ScheduledJob->Priority], ActiveJobFiber);
				}
			}
		} while (ActiveJobFiber);
	}
	return NULL;
}

void JobFiberProcedure(void *ParameterPointer) {
	JobFiberParameter *Parameter = (JobFiberParameter *)ParameterPointer;
	Job *ActiveJob;
	while (1) {
		ActiveJob = &Parameter->ScheduledJob;
		ActiveJob->Procedure(ActiveJob->Parameter);
		ActiveJob->Finished = 1;
		PlatformSwitchToFiber(&thread_local_jobsContext.worker_thread_fiber);
	}
}

JobDeclaration CreateJob(JobProcedure Procedure, void *Parameter) {
	return {
		.Procedure = Procedure,
		.Parameter = Parameter,
	};
}

void RunJobs(u32 JobCount, JobDeclaration *JobDeclarations, JobPriority Priority, JobCounter *Counter) {
	if (Counter) {
		Counter->UnfinishedJobCount = JobCount;
		Counter->WaitingJobFiber = NULL;
	}
	for (u32 I = 0; I < JobCount; I++) {
		Write(&jobsContext.JobQueues[Priority], (Job){
			.Procedure = JobDeclarations[I].Procedure,
			.Parameter = JobDeclarations[I].Parameter,
			.Priority = Priority,
			.CounterWaitingForThisJob = Counter,
			.Finished = 0,
		});
		PlatformSignalSemaphore(&jobsContext.JobsAvailableSemaphore);
	}
}

void ClearJobCounter(JobCounter *Counter) {
	Counter->UnfinishedJobCount = 0;
	Counter->WaitingJobFiber = NULL;
}

void WaitForJobCounter(JobCounter *Counter) {
	if (Counter->UnfinishedJobCount == 0) {
		return;
	}
	thread_local_jobsContext.waiting_job_counter = Counter;
	PlatformSwitchToFiber(&thread_local_jobsContext.worker_thread_fiber);
}

void InitializeJobs(GameState *GameState, JobProcedure InitialJobProcedure, void *InitialJobParameter) {
	for (u32 I = 0; I < JOB_FIBER_COUNT; I++) {
		JobFiber *NewFiber = allocate_struct(&GameState->permanent_arena, JobFiber);
		PlatformCreateFiber(&NewFiber->platformFiber, JobFiberProcedure, &NewFiber->parameter);
		NewFiber->next = jobsContext.IdleJobFiberList.head;
		jobsContext.IdleJobFiberList.head = NewFiber;
	}
	jobsContext.JobsAvailableSemaphore = PlatformCreateSemaphore(0);
	GameState->jobsContext.workerThreadCount = PlatformGetProcessorCount();
	jobsContext.WorkerThreads = allocate_array(&GameState->permanent_arena, WorkerThread, GameState->jobsContext.workerThreadCount);
	for (u32 i = 0; i < GameState->jobsContext.workerThreadCount; i++) {
		PlatformCreateThread(WorkerThreadProcedure, &jobsContext.WorkerThreads[i].Parameter);
		jobsContext.WorkerThreads[i].Parameter = (WorkerThreadParameter){
			.ThreadIndex = i,
		};
	}
	jobsContext.WorkerThreads = allocate_array(&GameState->permanent_arena, WorkerThread, GameState->jobsContext.workerThreadCount);
	for (u32 i = 0; i < GameState->jobsContext.workerThreadCount - 1; i++) {
		jobsContext.WorkerThreads[i].PlatformThread = PlatformCreateThread(WorkerThreadProcedure, &jobsContext.WorkerThreads[i].Parameter);
	}
	jobsContext.WorkerThreads[GameState->jobsContext.workerThreadCount - 1].PlatformThread = PlatformGetCurrentThread();
	for (u32 i = 0; i < GameState->jobsContext.workerThreadCount; i++) {
		PlatformSetThreadProcessorAffinity(jobsContext.WorkerThreads[i].PlatformThread, i);
	}
	JobDeclaration InitialJob = CreateJob(InitialJobProcedure, InitialJobParameter);
	RunJobs(1, &InitialJob, HIGH_PRIORITY_JOB, NULL);
	WorkerThreadProcedure(&jobsContext.WorkerThreads[GameState->jobsContext.workerThreadCount - 1].Parameter);
}

