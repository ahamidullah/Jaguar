#include "../Time.h"
#include "../Log.h"

bool operator>(PlatformTime a, const PlatformTime b)
{
	if (a.ts.tv_sec > b.ts.tv_sec)
	{
		return true;
	}
	else if (b.ts.tv_sec > a.ts.tv_sec)
	{
		return false;
	}
	else if (a.ts.tv_nsec > b.ts.tv_nsec)
	{
		return true;
	}
	else if (b.ts.tv_nsec > a.ts.tv_nsec)
	{
		return false;
	}
	return false;
}

bool operator<(const PlatformTime a, const PlatformTime b)
{
	if (a.ts.tv_sec < b.ts.tv_sec)
	{
		return true;
	}
	else if (b.ts.tv_sec < a.ts.tv_sec)
	{
		return false;
	}
	else if (a.ts.tv_nsec < b.ts.tv_nsec)
	{
		return true;
	}
	else if (b.ts.tv_nsec < a.ts.tv_nsec)
	{
		return false;
	}
	return false;
}

PlatformTime GetPlatformTime()
{
	auto time = PlatformTime{};
	clock_gettime(CLOCK_MONOTONIC_RAW, &time.ts);
	return time;
}

// Time in milliseconds.
f64 PlatformTimeDifference(PlatformTime start, PlatformTime end)
{
	return ((end.ts.tv_sec - start.ts.tv_sec) * 1000.0) + ((end.ts.tv_nsec - start.ts.tv_nsec) / 1000000.0);
}

void Sleep(s64 milliseconds)
{
	auto timespec = (struct timespec)
	{
		.tv_sec = milliseconds / 1000,
		.tv_nsec = (milliseconds % 1000) * 1000000,
	};
	if (nanosleep(&timespec, NULL))
	{
		LogPrint(ERROR_LOG, "nanosleep() ended early: %s.\n", GetPlatformError());
	}
}
