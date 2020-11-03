#pragma once

#include "Time.h"

namespace Time
{

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

}
