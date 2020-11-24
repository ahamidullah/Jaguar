#pragma once

#include "Time.h"

namespace time
{

struct Timer
{
	string::String name;
	Time start;
	s64 iteration;
	s64 runningSum;

	Duration Elapsed();
	void Print(s64 scale);
	void Reset();
	void Clear();
};

Timer NewTimer(string::String name);

}
