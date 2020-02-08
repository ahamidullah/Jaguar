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
#include "Jobs.h"

namespace Jobs {

constexpr auto MAX_JOBS_PER_QUEUE = 100;

#define JOB_FIBER_POINTER_SENTINEL ((JobFiber *)0xDEADBEEF)

struct Job {
	JobProcedure Procedure;
	void *Parameter;
	Priority QueuePriority;
	Counter *CounterWaitingForThisJob;
	bool Finished;
};

struct JobFiberParameter {
	Job ScheduledJob; // @TODO: Running job?
};

struct JobFiber {
	Platform::Fiber PlatformFiber;
	JobFiberParameter Parameter;
	struct JobFiber *Next;
};

struct WorkerThreadParameter {
	u32 ThreadIndex;
};

struct WorkerThread {
	Platform::ThreadHandle PlatformThread;
	WorkerThreadParameter Parameter;
};

struct Context {
	WorkerThread *WorkerThreads;
	JobFiber *IdleJobFiberList;
	AtomicRingBuffer<Job, MAX_JOBS_PER_QUEUE> JobQueues[PRIORITY_COUNT];
	AtomicRingBuffer<JobFiber *, MAX_JOBS_PER_QUEUE> ResumableJobQueues[PRIORITY_COUNT];
	Platform::Semaphore JobsAvailableSemaphore;
} jobsContext;

__thread struct {
	Platform::Fiber worker_thread_fiber;
	//Job_Fiber *active_job_fiber;
	Counter *waiting_job_counter;
} thread_local_jobsContext;

#define Atomic_Push_To_Front_Of_List(head, new_head) \
	do { \
		void *current_head; \
		do { \
			current_head = head; \
			new_head->Next = (decltype(new_head->Next))current_head; \
		} while (Platform::CompareAndSwapPointers((void *volatile *)&head, current_head, new_head) != current_head); \
	} while (0)

#define Atomic_Pop_From_Front_Of_List(head, result) \
	do { \
		void *current_head; \
		do { \
			current_head = head; \
			Assert(current_head); \
		} while (Platform::CompareAndSwapPointers((void *volatile *)&head, current_head, head->Next) != current_head); \
		result = (decltype(result))current_head; \
	} while (0)

#define Atomic_Write_To_Ring_Buffer(container, new_element) \
	do { \
		s32 current_write_index; \
		do { \
			/* @TODO: Some way to detect overwriting a job that isn't done yet (at least in debug mode). */ \
			current_write_index = container.write_index; \
		} while(Platform::CompareAndSwapS32(&container.write_index, current_write_index, (current_write_index + 1) % MAX_JOBS_PER_QUEUE) != current_write_index); \
		container.elements[current_write_index] = new_element; \
		container.ready[current_write_index] = true; \
	} while (0)

#define Atomic_Read_From_Ring_Buffer(container, result) \
	do { \
		result = NULL; \
		while (container.read_index != container.write_index) { \
			/* We need to do this a bit carefully to make sure we stop trying to get read an element if the ring buffer is emptied. */ \
			s32 read_index = container.read_index; \
			if (read_index != container.write_index) { \
				s32 job_index = Platform::CompareAndSwapS32(&container.read_index, read_index, (read_index + 1) % MAX_JOBS_PER_QUEUE); \
				if (job_index == read_index) { \
					while (!container.ready[job_index]) { \
						/**/ \
					} \
					result = &container.elements[job_index]; \
					break; \
				} \
			} \
		} \
	} while (0)

/*
Job *Get_Next_Job_From_Queue(Job_Queue *queue) {
	while (queue->read_index != queue->write_index) {
		// We need to do this a bit carefully to make sure we stop trying to get a job if the queue is emptied.
		s32 current_read_index = queue->read_index;
		if (current_read_index != queue->write_index) {
			s32 next_job_index = Platform_Compare_And_Swap_S32(&queue->read_index, current_read_index, (current_read_index + 1) % MAX_JOBS_PER_QUEUE);
			if (next_job_index == current_read_index) {
				return &queue->jobs[next_job_index];
			}
		}
	}
	return NULL;
}

Job *Get_Next_Job() {
	Job *job;
	for (u32 i = 0; i < JOB_PRIORITY_COUNT; i++) {
		if (jobsContext.job_queues[i].read_index != jobsContext.job_queues[i].write_index) {
			if ((job = Get_Next_Job_From_Queue(&jobsContext.job_queues[i]))) {
				return job;
			}
		}
	}
	return NULL;
}
*/

void *WorkerThreadProcedure(void *parameter) {
	Platform::ConvertThreadToFiber(&thread_local_jobsContext.worker_thread_fiber);
	thread_index = ((WorkerThreadParameter *)parameter)->ThreadIndex;
	JobFiber *ActiveJobFiber = NULL;
	while (1) {
		Platform::WaitOnSemaphore(&jobsContext.JobsAvailableSemaphore);
		do {
			ActiveJobFiber = NULL;
			// Get the next job to run. Prefer higher priority jobs and resumable jobs.
			for (s32 i = 0; i < PRIORITY_COUNT; i++) {
				if (Read(&jobsContext.ResumableJobQueues[i], &ActiveJobFiber)) {
					break;
				}
				// Avoid copying the Job twice by getting the idle job fiber before we know if we have a job.
				// If there are no more jobs left, just write the job fiber back to the idle job fiber list.
				Atomic_Pop_From_Front_Of_List(jobsContext.IdleJobFiberList, ActiveJobFiber);
				if (Read(&jobsContext.JobQueues[i], &ActiveJobFiber->Parameter.ScheduledJob)) {
					break;
				} else {
					Atomic_Push_To_Front_Of_List(jobsContext.IdleJobFiberList, ActiveJobFiber);
					ActiveJobFiber = NULL;
				}
			}
			if (!ActiveJobFiber) {
				break;
			}
			Platform::SwitchToFiber(&ActiveJobFiber->PlatformFiber);
			Job *ScheduledJob = &ActiveJobFiber->Parameter.ScheduledJob;
			if (ScheduledJob->Finished) {
				Atomic_Push_To_Front_Of_List(jobsContext.IdleJobFiberList, ActiveJobFiber);
				if (!ScheduledJob->CounterWaitingForThisJob) {
					continue;
				}
				// Check if the completion of this job caused the associated job counter to reach zero. If so, mark the job waiting on the counter as resumable.
				s32 UnfinishedJobCount = Platform::AtomicAddS32(&ScheduledJob->CounterWaitingForThisJob->UnfinishedJobCount, -1);
				if (UnfinishedJobCount > 0) {
					continue;
				}
				// Check to see if the parent job started waiting on the counter yet. If it hasn't then the job counter's waitingJobFiber will be NULL.
				JobFiber *WaitingJobFiber = (JobFiber *)Platform::FetchAndSetPointer((void *volatile *)&ScheduledJob->CounterWaitingForThisJob->WaitingJobFiber, JOB_FIBER_POINTER_SENTINEL);
				if (!WaitingJobFiber) {
					continue;
				}
				// OK, the job finished, the counter reached zero, and the parent job started waiting on the counter. Now we can mark the parent job as resumable.
				Write(&jobsContext.ResumableJobQueues[WaitingJobFiber->Parameter.ScheduledJob.QueuePriority], WaitingJobFiber);
			} else {
				if (Platform::CompareAndSwapPointers((void *volatile *)&thread_local_jobsContext.waiting_job_counter->WaitingJobFiber, NULL, ActiveJobFiber) == JOB_FIBER_POINTER_SENTINEL) {
					// The waitingJobFiber was not NULL, hence all dependency jobs already finished. This job can resume immediately.
					Write(&jobsContext.ResumableJobQueues[ScheduledJob->QueuePriority], ActiveJobFiber);
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
		Platform::SwitchToFiber(&thread_local_jobsContext.worker_thread_fiber);
	}
}

JobDeclaration Create(JobProcedure Procedure, void *Parameter) {
	return {
		.Procedure = Procedure,
		.Parameter = Parameter,
	};
}

void Run(u32 JobCount, JobDeclaration *JobDeclarations, Priority JobPriority, Counter *JobCounter) {
	if (JobCounter) {
		JobCounter->UnfinishedJobCount = JobCount;
		JobCounter->WaitingJobFiber = NULL;
	}
	for (u32 I = 0; I < JobCount; I++) {
		Write(&jobsContext.JobQueues[JobPriority], (Job){
			.Procedure = JobDeclarations[I].Procedure,
			.Parameter = JobDeclarations[I].Parameter,
			.QueuePriority = JobPriority,
			.CounterWaitingForThisJob = JobCounter,
			.Finished = 0,
		});
		Platform::SignalSemaphore(&jobsContext.JobsAvailableSemaphore);
	}
}

void ClearCounter(Counter *JobCounter) {
	JobCounter->UnfinishedJobCount = 0;
	JobCounter->WaitingJobFiber = NULL;
}

void WaitForCounter(Counter *JobCounter) {
	if (JobCounter->UnfinishedJobCount == 0) {
		return;
	}
	thread_local_jobsContext.waiting_job_counter = JobCounter;
	Platform::SwitchToFiber(&thread_local_jobsContext.worker_thread_fiber);
}

void Initialize(GameState *GameState, JobProcedure InitialJobProcedure, void *InitialJobParameter) {
	for (u32 I = 0; I < JOB_FIBER_COUNT; I++) {
		JobFiber *NewFiber = allocate_struct(&GameState->permanent_arena, JobFiber);
		Platform::CreateFiber(&NewFiber->PlatformFiber, JobFiberProcedure, &NewFiber->Parameter);
		NewFiber->Next = jobsContext.IdleJobFiberList;
		jobsContext.IdleJobFiberList = NewFiber;
	}
	jobsContext.JobsAvailableSemaphore = Platform::CreateSemaphore(0);
	GameState->jobsContext.workerThreadCount = Platform::GetProcessorCount();
	jobsContext.WorkerThreads = allocate_array(&GameState->permanent_arena, WorkerThread, GameState->jobsContext.workerThreadCount);
	for (u32 i = 0; i < GameState->jobsContext.workerThreadCount; i++) {
		Platform::CreateThread(WorkerThreadProcedure, &jobsContext.WorkerThreads[i].Parameter);
		jobsContext.WorkerThreads[i].Parameter = (WorkerThreadParameter){
			.ThreadIndex = i,
		};
	}
	jobsContext.WorkerThreads = allocate_array(&GameState->permanent_arena, WorkerThread, GameState->jobsContext.workerThreadCount);
	for (u32 i = 0; i < GameState->jobsContext.workerThreadCount - 1; i++) {
		jobsContext.WorkerThreads[i].PlatformThread = Platform::CreateThread(WorkerThreadProcedure, &jobsContext.WorkerThreads[i].Parameter);
	}
	jobsContext.WorkerThreads[GameState->jobsContext.workerThreadCount - 1].PlatformThread = Platform::GetCurrentThread();
	for (u32 i = 0; i < GameState->jobsContext.workerThreadCount; i++) {
		Platform::SetThreadProcessorAffinity(jobsContext.WorkerThreads[i].PlatformThread, i);
	}
	JobDeclaration InitialJob = Create(InitialJobProcedure, InitialJobParameter);
	Run(1, &InitialJob, HIGH_PRIORITY, NULL);
	WorkerThreadProcedure(&jobsContext.WorkerThreads[GameState->jobsContext.workerThreadCount - 1].Parameter);
}

} // namespace Jobs
