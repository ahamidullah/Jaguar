#pragma once

#if defined(__linux__)
	#include "Linux/Time.h"
#else
	#error Unsupported platform.
#endif

struct String;

struct Timer
{
	Time start;
	f64 runningSum;
	u64 iteration;
	String name;
};

#define StartTimer(timerName) Timer TIMER_#timerName = {.name = #timerName, .start = GetTime(), .iteration = 1}
#define PrintTimer(timerName) PrintTimerActual(&TIMER_##timerName)
