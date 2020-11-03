#pragma once

namespace Time
{

struct Duration
{
	s64 nanoseconds;

	bool operator>(Duration d);
	bool operator<(Duration d);
	Duration operator-(Duration d);
	s64 Hours();
	s64 Minutes();
	s64 Seconds();
	s64 Milliseconds();
	s64 Nanoseconds();
};

}
