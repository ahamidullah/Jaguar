#pragma once

#include <time.h>

struct PlatformTime
{
	timespec ts;
};

#define PLATFORM_TIME_ERROR (PlatformTime{.ts{0, -1}})

PlatformTime GetPlatformTime();
f64 PlatformTimeDifference(PlatformTime start, PlatformTime end);
void Sleep(u32 milliseconds);
bool operator>(const PlatformTime &a, const PlatformTime &b);
bool operator<(const PlatformTime &a, const PlatformTime &b);
