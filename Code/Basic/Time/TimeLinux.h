#pragma once

#include "Duration.h"
#include "Date.h"
#include "Basic/PCH.h"
#include "Common.h"

namespace Time
{

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
	s64 Nanosecond();
};

Time Now();
void Sleep(s64 nsec);

}
