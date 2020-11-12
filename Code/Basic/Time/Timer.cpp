#include "Timer.h"
#include "Basic/String.h"

namespace Time
{

Timer NewTimer(string::String name)
{
	return
	{
		.name = name,
		.start = Now(),
		.iteration = 1,
	};
}

Duration Timer::Elapsed()
{
	return Now() - this->start;
}

void Timer::Print(s64 scale)
{
	auto delta = Now() - this->start;
	this->runningSum += delta.Nanoseconds();
	auto unit = string::String{};
	switch (scale)
	{
	case Nanosecond:
	{
		unit = "ns";
	} break;
	case Millisecond:
	{
		unit = "ms";
	} break;
	case Second:
	{
		unit = "s";
	} break;
	case Minute:
	{
		unit = "m";
	} break;
	case Hour:
	{
		unit = "m";
	} break;
	default:
	{
		LogError("Time", "Unknown time scale %ld.", scale);
		unit = "?";
	};
	};
	ConsolePrint("%k: %ld%k, avg: %f%k\n", this->name, delta.Nanoseconds() / scale, unit, (f32)(this->runningSum / scale) / (f32)this->iteration, unit);
	this->iteration += 1;
}

void Timer::Reset()
{
	this->start = Now();
}

void Timer::Clear()
{
	this->start = Now();
	this->runningSum = 0;
	this->iteration = 1;
}

}
