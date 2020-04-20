#pragma once

constexpr auto JOB_FIBER_COUNT = 160;

enum JobPriority {
	HIGH_PRIORITY_JOB,
	NORMAL_PRIORITY_JOB,
	LOW_PRIORITY_JOB,
	JOB_PRIORITY_COUNT
};

struct JobFiber;

struct JobCounter {
	volatile s32 unfinishedJobCount;
	JobFiber *waitingJobFiber;
};

typedef void (*JobProcedure)(void *);

struct JobDeclaration {
	JobProcedure procedure;
	void *parameter;
};

void InitializeJobs(JobProcedure initialJobProcedure, void *initialJobParameter);
JobDeclaration CreateJob(JobProcedure procedure, void *parameter);
void RunJobs(u32 JobCount, JobDeclaration *JobDeclarations, JobPriority priority, JobCounter *counter);
void WaitForJobCounter(JobCounter *counter);
void ClearJobCounter(JobCounter *counter);
size_t GetWorkerThreadCount();
