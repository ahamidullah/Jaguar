#pragma once

#if defined(__linux__)
	#include "Linux/Time.h"
#else
	#error Unsupported platform.
#endif
#include "String.h"

struct Timer
{
	String name;
	Time start;
	s64 iteration;
	s64 runningSum;

	Duration Elapsed();
	void Print(s64 scale);
	void Reset();
	void Clear();
};

Timer NewTimer(String name);

s64 SecondsToNanoseconds(s64 s);
s64 NanosecondsToMilliseconds(s64 n);
s64 NanosecondsToSeconds(s64 n);
s64 NanosecondsToMinutes(s64 n);
s64 NanosecondsToHours(s64 n);

const auto NanosecondScale = 1LL;
const auto MillisecondScale = 1000000LL;
const auto SecondScale = 1000000000LL;
const auto MinuteScale = 60000000000LL;
const auto HourScale = 1200000000000LL;
