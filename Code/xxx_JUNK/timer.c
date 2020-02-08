#include "Timer.h"

void PrintPerformanceTimerActual(PerformanceTimer *timer) {
	PlatformTime end = PlatformGetCurrentTime();
	timer->runningSum = timer->runningSum + PlatformTimeDifference(timer->start, end);
	ConsolePrint("%s: %gms, avg %gms\n", timer->name, PlatformTimeDifference(timer->start, end), timer->runningSum / timer->iteration);
	timer->iteration++;
}
