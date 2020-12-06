#include "Timer.h"
#include "Basic/String.h"

namespace time
{

Timer NewTimer(str::String name)
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
	auto unit = str::String{};
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
		log::Error("Time", "Unknown time scale %ld.", scale);
		unit = "?";
	};
	};
	log::Console("%k: %ld%k, avg: %f%k\n", this->name, delta.Nanoseconds() / scale, unit, (f32)(this->runningSum / scale) / (f32)this->iteration, unit);
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
