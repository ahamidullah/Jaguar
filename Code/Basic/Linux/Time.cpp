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

PlatformTime CurrentTime()
{
	auto t = PlatformTime{};
	clock_gettime(CLOCK_MONOTONIC_RAW, &t.ts);
	return t;
}

void FormatTimestamp(StringBuilder *sb, PlatformTime t)
{
	FormatString(sb, "%jd", (SignedIntMax)t.ts.tv_sec);
}

// @TODO: Handle the format properly!
void FormatTime(StringBuilder *sb, String fmt, PlatformTime t)
{
	auto tm = localtime(&t.ts.tv_sec);
	FormatString(sb, fmt, tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, t.ts.tv_nsec / 1000000);
}

// Time in milliseconds.
f64 PlatformTimeDifference(PlatformTime a, PlatformTime b)
{
	return ((b.ts.tv_sec - a.ts.tv_sec) * 1000.0) + ((b.ts.tv_nsec - a.ts.tv_nsec) / 1000000.0);
}

void Sleep(s64 msec)
{
	auto ts = (struct timespec)
	{
		.tv_sec = msec / 1000,
		.tv_nsec = (msec % 1000) * 1000000,
	};
	if (nanosleep(&ts, NULL))
	{
		LogPrint(LogLevelError, "Time", "nanosleep() ended early: %k.\n", GetPlatformError());
	}
}
