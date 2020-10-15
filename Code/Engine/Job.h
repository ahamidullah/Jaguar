#pragma once

#include "Basic/Fiber.h"
#include "Common.h"

constexpr auto JobFiberCount = 160;

enum JobPriority
{
	HighJobPriority,
	NormalJobPriority,
	LowJobPriority,
	JobPriorityCount
};

struct JobCounter;

typedef void (*JobProcedure)(void *);

struct QueuedJob
{
	JobProcedure procedure;
	void *parameter;
	JobCounter *waitingCounter;
};

struct JobFiberParameter
{
	JobPriority priority;
	JobProcedure procedure;
	void *parameter;
	bool finished;
	JobCounter *waitingCounter; // The job counter waiting on this job to complete. Can be NULL.
	s64 threadIndex;
};

struct JobFiber
{
	Fiber platformFiber;
	JobFiberParameter parameter;
};

struct JobCounter
{
	s64 jobCount;
	s64 unfinishedJobCount;
	Array<JobFiber *> waitingFibers;

	void Wait();
	void Reset();
	void Free();
};

struct JobDeclaration
{
	JobProcedure procedure;
	void *parameter;
};

void InitializeJobs(JobProcedure init, void *param);
JobDeclaration NewJobDeclaration(JobProcedure proc, void *param);
void RunJobs(ArrayView<JobDeclaration> d, JobPriority p, JobCounter **c);
s64 WorkerThreadCount();
