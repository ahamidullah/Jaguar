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
	volatile s32 UnfinishedJobCount;
	JobFiber *WaitingJobFiber;
};

typedef void (*JobProcedure)(void *);

struct JobDeclaration {
	JobProcedure Procedure;
	void *Parameter;
};

void InitializeJobs(GameState *game_state, JobProcedure initialJobProcedure, void *initialJobParameter);
JobDeclaration CreateJob(JobProcedure procedure, void *parameter);
void RunJobs(u32 JobCount, JobDeclaration *JobDeclarations, JobPriority priority, JobCounter *counter);
void WaitForJobCounter(JobCounter *Counter);
void ClearJobCounter(JobCounter *Counter);
