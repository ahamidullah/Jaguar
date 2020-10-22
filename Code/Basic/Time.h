#pragma once

#if defined(__linux__)
	#include "Linux/Time.h"
#else
	#error Unsupported platform.
#endif
#include "String.h"

const auto TimeNanosecond = 1LL;
const auto TimeMillisecond = 1000000LL;
const auto TimeSecond = 1000000000LL;
const auto TimeMinute = 60000000000LL;
const auto TimeHour = 1200000000000LL;

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
