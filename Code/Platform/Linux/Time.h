#pragma once

typedef struct timespec PlatformTime;

PlatformTime PlatformGetCurrentTime();
f64 PlatformTimeDifference(PlatformTime start, PlatformTime end);
void PlatformSleep(u32 milliseconds);
