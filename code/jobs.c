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

#define JOB_FIBER_COUNT 160
#define MAX_JOBS_PER_QUEUE 100
#define JOB_FIBER_POINTER_SENTINEL ((Job_Fiber *)0xDEADBEEF)

typedef enum {
	HIGH_JOB_PRIORITY,
	NORMAL_JOB_PRIORITY,
	LOW_JOB_PRIORITY,
	JOB_PRIORITY_COUNT
} Job_Priority;

typedef struct Job Job;
typedef struct Jobs_Context Jobs_Context;
typedef void (*Job_Procedure)(void *);

typedef struct Job_Fiber_Parameter {
	Job *scheduled_job; // @TODO: Running job?
} Job_Fiber_Parameter;

typedef struct Job_Fiber {
	Platform_Fiber platform_fiber;
	Job_Fiber_Parameter parameter;
	struct Job_Fiber *next;
} Job_Fiber;

typedef struct Job_Counter {
	volatile s32 unfinished_job_count;
	Job_Fiber *waiting_job_fiber;
} Job_Counter;

typedef struct Job_Declaration {
	Job_Procedure procedure;
	void *parameter;
} Job_Declaration;

typedef struct Job {
	Job_Procedure procedure;
	void *parameter;
	Job_Priority priority;
	Job_Counter *counter;
	u8 finished;
} Job;

typedef struct Job_Queue {
	Job elements[MAX_JOBS_PER_QUEUE];
	Ring_Buffer(MAX_JOBS_PER_QUEUE);
	//Ring_Buffer ring_buffer;
} Job_Queue;

typedef struct Resumable_Job_Queue {
	Job_Fiber *elements[MAX_JOBS_PER_QUEUE];
	Ring_Buffer(MAX_JOBS_PER_QUEUE);
	//Ring_Buffer ring_buffer;
} Resumable_Job_Queue;

typedef struct Worker_Thread_Parameter {
	u32 thread_index;
} Worker_Thread_Parameter;

typedef struct Worker_Thread {
	Platform_Thread_Handle platform_thread;
	Worker_Thread_Parameter parameter;
} Worker_Thread;

typedef struct Jobs_Context {
	Worker_Thread *worker_threads;
	Job_Fiber *idle_job_fiber_list;
	Job_Queue job_queues[JOB_PRIORITY_COUNT];
	Resumable_Job_Queue resumable_job_queues[JOB_PRIORITY_COUNT];
	Platform_Semaphore jobs_available_semaphore;
} Jobs_Context;

__thread struct {
	Platform_Fiber worker_thread_fiber;
	//Job_Fiber *active_job_fiber;
	Job_Counter *waiting_job_counter;
} thread_local_jobs_context;

Jobs_Context jobs_context;

#define Atomic_Push_To_Front_Of_List(head, new_head) \
	do { \
		void *current_head; \
		do { \
			current_head = head; \
			new_head->next = current_head; \
		} while (Platform_Compare_And_Swap_Pointers((void *volatile *)&head, current_head, new_head) != current_head); \
	} while (0)

#define Atomic_Pop_From_Front_Of_List(head, result) \
	do { \
		void *current_head; \
		do { \
			current_head = head; \
			Assert(current_head); \
		} while (Platform_Compare_And_Swap_Pointers((void *volatile *)&head, current_head, head->next) != current_head); \
		result = current_head; \
	} while (0)

#define Atomic_Write_To_Ring_Buffer(container, new_element) \
	do { \
		s32 current_write_index; \
		do { \
			/* @TODO: Some way to detect overwriting a job that isn't done yet (at least in debug mode). */ \
			current_write_index = container.write_index; \
		} while(Platform_Compare_And_Swap_S32(&container.write_index, current_write_index, (current_write_index + 1) % MAX_JOBS_PER_QUEUE) != current_write_index); \
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
				s32 job_index = Platform_Compare_And_Swap_S32(&container.read_index, read_index, (read_index + 1) % MAX_JOBS_PER_QUEUE); \
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
		if (jobs_context.job_queues[i].read_index != jobs_context.job_queues[i].write_index) {
			if ((job = Get_Next_Job_From_Queue(&jobs_context.job_queues[i]))) {
				return job;
			}
		}
	}
	return NULL;
}
*/

void *Worker_Thread_Procedure(void *parameter) {
	Platform_Convert_Thread_To_Fiber(&thread_local_jobs_context.worker_thread_fiber);
	thread_index = ((Worker_Thread_Parameter *)parameter)->thread_index;
	Job_Fiber *scheduled_job_fiber = NULL;
	while (1) {
		Platform_Wait_Semaphore(&jobs_context.jobs_available_semaphore);
		do {
			scheduled_job_fiber = NULL;
			// Get the next job to run. Prefer higher priority jobs and resumable jobs.
			{
				Job_Fiber **resumable_job_fiber = NULL;
				Job *new_job = NULL;
				for (s32 i = 0; i < JOB_PRIORITY_COUNT; i++) {
					Atomic_Read_From_Ring_Buffer(jobs_context.resumable_job_queues[i], resumable_job_fiber);
					if (resumable_job_fiber) {
						scheduled_job_fiber = *resumable_job_fiber;
						break;
					}
					Atomic_Read_From_Ring_Buffer(jobs_context.job_queues[i], new_job);
					if (new_job) {
						Atomic_Pop_From_Front_Of_List(jobs_context.idle_job_fiber_list, scheduled_job_fiber);
						scheduled_job_fiber->parameter.scheduled_job = new_job;
						break;
					}
				}
			}
			if (!scheduled_job_fiber) {
				break;
			}
			Platform_Switch_To_Fiber(&scheduled_job_fiber->platform_fiber);
			Job *scheduled_job = scheduled_job_fiber->parameter.scheduled_job;
			if (scheduled_job->finished) {
				Atomic_Push_To_Front_Of_List(jobs_context.idle_job_fiber_list, scheduled_job_fiber);
				if (!scheduled_job->counter) {
					continue;
				}
				// Check if the completion of this job caused the associated job counter to reach zero. If so, mark the job waiting on the counter as resumable.
				s32 unfinished_job_count = Platform_Atomic_Add_S32(&scheduled_job->counter->unfinished_job_count, -1);
				if (unfinished_job_count > 0) {
					continue;
				}
				// Check to see if the parent job started waiting on the counter yet. If it hasn't then the job counter's waiting_job_fiber will be NULL.
				Job_Fiber *waiting_job_fiber = Platform_Fetch_And_Set_Pointer((void *volatile *)&scheduled_job->counter->waiting_job_fiber, JOB_FIBER_POINTER_SENTINEL);
				if (!waiting_job_fiber) {
					continue;
				}
				// OK, the job finished, the counter reached zero, and the parent job started waiting on the counter. Now we can mark the parent job as resumable.
				Atomic_Write_To_Ring_Buffer(jobs_context.resumable_job_queues[waiting_job_fiber->parameter.scheduled_job->priority], waiting_job_fiber);
			} else {
				if (Platform_Compare_And_Swap_Pointers((void *volatile *)&thread_local_jobs_context.waiting_job_counter->waiting_job_fiber, NULL, scheduled_job_fiber) == JOB_FIBER_POINTER_SENTINEL) {
					// The waiting_job_fiber was not NULL, hence all dependency jobs already finished. This job can resume immediately.
					Atomic_Write_To_Ring_Buffer(jobs_context.resumable_job_queues[scheduled_job->priority], scheduled_job_fiber);
				}
			}
		} while (scheduled_job_fiber);
	}
	return NULL;
}

void Job_Fiber_Procedure(void *parameter) {
	Job_Fiber_Parameter *job_fiber_parameter = (Job_Fiber_Parameter *)parameter;
	Job *job;
	while (1) {
		job = job_fiber_parameter->scheduled_job;
		job->procedure(job->parameter);
		job->finished = 1;
		Platform_Switch_To_Fiber(&thread_local_jobs_context.worker_thread_fiber);
	}
}

Job_Declaration Create_Job(Job_Procedure procedure, void *parameter) {
	return (Job_Declaration){
		.procedure = procedure,
		.parameter = parameter,
	};
}

void Run_Jobs(u32 count, Job_Declaration *job_declarations, Job_Priority priority, Job_Counter *counter) {
	if (counter) {
		counter->unfinished_job_count = count;
		counter->waiting_job_fiber = NULL;
	}
	for (u32 i = 0; i < count; i++) {
		Job new_job = {
			.procedure = job_declarations[i].procedure,
			.parameter = job_declarations[i].parameter,
			.finished = 0,
			.priority = priority,
			.counter = counter,
		};
		Atomic_Write_To_Ring_Buffer(jobs_context.job_queues[priority], new_job);
		Platform_Post_Semaphore(&jobs_context.jobs_available_semaphore);
	}
}

void Clear_Job_Counter(Job_Counter *counter) {
	counter->unfinished_job_count = 0;
	counter->waiting_job_fiber = NULL;
}

void Wait_For_Job_Counter(Job_Counter *counter) {
	if (counter->unfinished_job_count == 0) {
		return;
	}
	thread_local_jobs_context.waiting_job_counter = counter;
	Platform_Switch_To_Fiber(&thread_local_jobs_context.worker_thread_fiber);
}

void Initialize_Jobs(Game_State *game_state, Job_Procedure initial_job_procedure, void *initial_job_parameter) {
	for (u32 i = 0; i < JOB_FIBER_COUNT; i++) {
		Job_Fiber *job_fiber = allocate_struct(&game_state->permanent_arena, Job_Fiber);
		Platform_Create_Fiber(&job_fiber->platform_fiber, Job_Fiber_Procedure, &job_fiber->parameter);
		job_fiber->next = jobs_context.idle_job_fiber_list;
		jobs_context.idle_job_fiber_list = job_fiber;
	}
	jobs_context.jobs_available_semaphore = Platform_Create_Semaphore(0);
	game_state->jobs_context.worker_thread_count = Platform_Get_Processor_Count();
	jobs_context.worker_threads = allocate_array(&game_state->permanent_arena, Worker_Thread, game_state->jobs_context.worker_thread_count);
	for (u32 i = 0; i < game_state->jobs_context.worker_thread_count; i++) {
		Platform_Create_Thread(Worker_Thread_Procedure, &jobs_context.worker_threads[i].parameter);
		jobs_context.worker_threads[i].parameter = (Worker_Thread_Parameter){
			.thread_index = i,
		};
	}
	jobs_context.worker_threads = allocate_array(&game_state->permanent_arena, Platform_Thread_Handle, game_state->jobs_context.worker_thread_count);
	for (u32 i = 0; i < game_state->jobs_context.worker_thread_count - 1; i++) {
		jobs_context.worker_threads[i].platform_thread = Platform_Create_Thread(Worker_Thread_Procedure, &jobs_context.worker_threads[i].parameter);
	}
	jobs_context.worker_threads[game_state->jobs_context.worker_thread_count - 1].platform_thread = Platform_Get_Current_Thread();
	for (u32 i = 0; i < game_state->jobs_context.worker_thread_count; i++) {
		Platform_Set_Thread_Processor_Affinity(jobs_context.worker_threads[i].platform_thread, i);
	}
	Job_Declaration initial_job = Create_Job(initial_job_procedure, initial_job_parameter);
	Job_Counter counter;
	Run_Jobs(1, &initial_job, HIGH_JOB_PRIORITY, &counter);
	Worker_Thread_Procedure(&jobs_context.worker_threads[game_state->jobs_context.worker_thread_count - 1].parameter);
}

/*
Job *get_next_job(Job_Queue *jq) {
	if (jq->read_head != jq->write_head) {
		u32 original_read_head = jq->read_head;
		if (original_read_head != jq->write_head) {
			//u32 next_job_to_do = __sync_val_compare_and_swap(&jq->read_head, original_read_head, (original_read_head + 1) % MAX_JOBS);
			u32 next_job_to_do = platform_compare_and_swap(&jq->read_head, original_read_head, (original_read_head + 1) % MAX_JOBS);
			if (next_job_to_do == original_read_head) {
				return &jq->jobs[next_job_to_do];
			}
		}
	}
	return NULL;
}

void Do_All_Jobs(Job_Queue *jq) {
	for (Job *j = get_next_job(jq); j; j = get_next_job(jq)) {
		j->do_job_callback(j->job_data);
		++jq->num_jobs_completed;
	}
}

void *job_thread_start(void *job_thread_data) {
	Job_Queue *jq = (Job_Queue *)job_thread_data;
	while (true) {
		do_all_jobs(jq);
		platform_wait_semaphore(&jq->semaphore);
	}
}
*/
