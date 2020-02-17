#pragma once

struct PerformanceTimer
{
	PlatformTime start;
	f64 runningSum;
	u64 iteration;
	const char *name;
};

#define StartPerformanceTimer(timerName) PerformanceTimer TIMER_#timerName = {.name = #timerName, .start = GetPlatformTime(), .iteration = 1}

#define PrintPerformanceTimer(timerName) PrintPerformanceTimerActual(&TIMER_##timerName)
