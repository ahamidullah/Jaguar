#pragma once

#include "../PCH.h"
#include "../String.h"
#include "Common.h"

struct Date
{
	s64 year;
	s64 month;
	s64 day;
	s64 hour;
	s64 minute;
	s64 second;
	s64 millisecond;
	s64 nanosecond;
};

struct Duration
{
	s64 nanoseconds;

	bool operator>(Duration d);
	bool operator<(Duration d);
	Duration operator-(Duration d);
	s64 Hour();
	s64 Minute();
	s64 Second();
	s64 Millisecond();
	s64 Nanosecond();
};

struct PlatformTime
{
	timespec ts;

	bool operator>(PlatformTime t);
	bool operator<(PlatformTime t);
	Duration operator-(PlatformTime t);
	Date Date();
	s64 Year();
	s64 Month();
	s64 Day();
	s64 Hour();
	s64 Minute();
	s64 Second();
	s64 Millisecond();
	s64 Nanosecond();
};

PlatformTime CurrentTime();
void Sleep(s64 msec);
