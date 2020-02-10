#include "Platform/Time.h"

PlatformTime PlatformGetCurrentTime()
{
	PlatformTime time;
	clock_gettime(CLOCK_MONOTONIC_RAW, &time);
	return time;
}

// Time in milliseconds.
f64 PlatformTimeDifference(PlatformTime start, PlatformTime end)
{
	return ((end.tv_sec - start.tv_sec) * 1000.0) + ((end.tv_nsec - start.tv_nsec) / 1000000.0);
}

void PlatformSleep(u32 milliseconds)
{
	struct timespec timespec = {
		.tv_sec = milliseconds / 1000,
		.tv_nsec = (milliseconds % 1000) * 1000000,
	};
	if (nanosleep(&timespec, NULL))
	{
		LogPrint(ERROR_LOG, "nanosleep() ended early: %s.", PlatformGetError());
	}
}
