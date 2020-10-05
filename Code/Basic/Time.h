#pragma once

#if defined(__linux__)
	#include "Linux/Time.h"
#else
	#error Unsupported platform.
#endif

struct String;

struct Timer
{
	String name;
	Time start;
	s64 iteration;
	s64 runningSum;

	Duration Elapsed();
	void Print();
	void Reset();
};

Timer NewTimer(String name);

s64 SecondsToNanoseconds(s64 s);
s64 NanosecondsToMilliseconds(s64 n);
s64 NanosecondsToSeconds(s64 n);
s64 NanosecondsToMinutes(s64 n);
s64 NanosecondsToHours(s64 n);
