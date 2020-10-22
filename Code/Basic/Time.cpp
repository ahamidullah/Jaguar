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
	case TimeNanosecond:
	{
		unit = "ns";
	} break;
	case TimeMillisecond:
	{
		unit = "ms";
	} break;
	case TimeSecond:
	{
		unit = "s";
	} break;
	case TimeMinute:
	{
		unit = "m";
	} break;
	case TimeHour:
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
