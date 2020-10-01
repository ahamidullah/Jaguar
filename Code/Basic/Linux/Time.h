#pragma once

#include "../PCH.h"
#include "Common.h"

#define SecondsToNanoseconds(s) (s * 1000000000)
#define NanosecondsToMilliseconds(n) (n / 1000000)
#define NanosecondsToSeconds(n) (NanosecondsToMilliseconds(n) / 1000)
#define NanosecondsToMinutes(n) (NanosecondsToSeconds(n) / 60)
#define NanosecondsToHours(n) (NanosecondsToMinutes(n) / 60)

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

struct Time
{
	timespec ts;

	bool operator>(Time t);
	bool operator<(Time t);
	Duration operator-(Time t);
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

Time CurrentTime();
void Sleep(s64 msec);
