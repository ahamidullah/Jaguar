#pragma once

#include <time.h>

struct PlatformTime
{
	timespec ts;
};

bool operator>(PlatformTime &a, PlatformTime &b);
bool operator<(PlatformTime &a, PlatformTime &b);

PlatformTime GetPlatformTime();
f64 PlatformTimeDifference(PlatformTime start, PlatformTime end);
void Sleep(s64 milliseconds);
