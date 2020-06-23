#pragma once

#include "../PCH.h"
#include "../String.h"

#include "Code/Common.h"

struct PlatformTime
{
	timespec ts;
};

bool operator>(PlatformTime a, PlatformTime b);
bool operator<(PlatformTime a, PlatformTime b);

PlatformTime CurrentTime();
void FormatTime(StringBuilder *sb, String fmt, PlatformTime t);
void FormatTimestamp(StringBuilder *sb, PlatformTime t);
f64 PlatformTimeDifference(PlatformTime a, PlatformTime b);
void Sleep(s64 msec);
