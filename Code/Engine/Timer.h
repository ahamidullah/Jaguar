#pragma once

#include "Code/Basic/Time.h"

struct Timer
{
	PlatformTime start;
	f64 runningSum;
	u64 iteration;
	const char *name;
};

#define StartTimer(timerName) Timer TIMER_#timerName = {.name = #timerName, .start = GetPlatformTime(), .iteration = 1}
#define PrintTimer(timerName) PrintTimerActual(&TIMER_##timerName)
