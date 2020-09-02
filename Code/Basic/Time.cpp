#include "Time.h"
#include "Log.h"

void PrintTimerActual(Timer *t)
{
	auto end = XCurrentTime();
	auto delta = t->start - end;
	t->runningSum += delta.Millisecond();
	ConsolePrint("%s: %gms, avg %gms\n", t->name, delta.Millisecond(), t->runningSum / t->iteration);
	t->iteration += 1;
}
