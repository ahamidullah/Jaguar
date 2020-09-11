#pragma once

#include "Basic/Fiber.h"
#include "Common.h"

constexpr auto JobFiberCount = 160;

enum JobPriority
{
	HighPriorityJob,
	NormalPriorityJob,
	LowPriorityJob,
	JobPriorityCount
};

typedef void (*JobProcedure)(void *);

struct Job
{
	JobProcedure procedure;
	void *parameter;
	JobPriority priority;
	//JobCounter *waitingCounter; // The job counter waiting on this job to complete. Can be NULL.
	//bool finished;
};

struct JobFiberParameter
{
	Job scheduledJob; // @TODO: Running job?
};

struct JobFiber
{
	Fiber platformFiber;
	JobFiberParameter parameter;
	JobFiber *next;
};

struct JobCounter
{
	//volatile s64 unfinishedJobCount;
	//JobFiber *waitingJobFiber;
	// Array<JobFiber *> waitingFibers;
};

struct JobDeclaration
{
	JobProcedure procedure;
	void *parameter;
};

void InitializeJobs(JobProcedure init, void *params);
//JobDeclaration CreateJob(JobProcedure procedure, void *parameter);
//void RunJobs(u32 JobCount, JobDeclaration *JobDeclarations, JobPriority priority, JobCounter *counter);
//void WaitForJobCounter(JobCounter *counter);
//void ClearJobCounter(JobCounter *counter);
s64 GetWorkerThreadCount();
//u32 GetThreadIndex();
