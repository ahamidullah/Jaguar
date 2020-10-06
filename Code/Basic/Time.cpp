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

void Timer::Print()
{
	auto delta = CurrentTime() - this->start;
	this->runningSum += delta.Millisecond();
	ConsolePrint("%k: %ldns %fms\n", this->name, delta.Nanosecond(), (f32)this->runningSum / (f32)this->iteration);
	this->iteration += 1;
}

void Timer::Reset()
{
	this->start = CurrentTime();
	this->runningSum = 0;
	this->iteration = 0;
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
