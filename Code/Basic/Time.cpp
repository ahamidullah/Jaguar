#include "Time.h"
#include "Log.h"

Timer NewTimer(String name)
{
	return
	{
		.name = name,
		.start = CurrentTime(),
		.iteration = 1,
	};
}

Duration Timer::Elapsed()
{
	return CurrentTime() - this->start;
}

void Timer::Print(s64 scale)
{
	auto delta = CurrentTime() - this->start;
	this->runningSum += delta.Nanosecond();
	auto unit = String{};
	switch (scale)
	{
	case NanosecondScale:
	{
		unit = "ns";
	} break;
	case MillisecondScale:
	{
		unit = "ms";
	} break;
	case SecondScale:
	{
		unit = "s";
	} break;
	case MinuteScale:
	{
		unit = "m";
	} break;
	case HourScale:
	{
		unit = "m";
	} break;
	default:
	{
		LogError("Time", "Unknown time scale %ld.", scale);
		unit = "?";
	};
	};
	ConsolePrint("%k: %ld%k, avg: %f%k\n", this->name, delta.Nanosecond() / scale, unit, (f32)(this->runningSum / scale) / (f32)this->iteration, unit);
	this->iteration += 1;
}

void Timer::Reset()
{
	this->start = CurrentTime();
}

void Timer::Clear()
{
	this->start = CurrentTime();
	this->runningSum = 0;
	this->iteration = 1;
}

s64 SecondsToNanoseconds(s64 s)
{
	return 1000000000 * s;
}

s64 NanosecondsToMilliseconds(s64 n)
{
	return n / 1000000;
}

s64 NanosecondsToSeconds(s64 n)
{
	return NanosecondsToMilliseconds(n) / 1000;
}

s64 NanosecondsToMinutes(s64 n)
{
	return NanosecondsToSeconds(n) / 60;
}

s64 NanosecondsToHours(s64 n)
{
	return NanosecondsToMinutes(n) / 60;
}
