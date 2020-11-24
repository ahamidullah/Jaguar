#include "Time.h"
#include "Basic/Log.h"

namespace time
{

bool Time::operator>(Time t)
{
	if (this->ts.tv_sec > t.ts.tv_sec)
	{
		return true;
	}
	else if (t.ts.tv_sec > this->ts.tv_sec)
	{
		return false;
	}
	else if (this->ts.tv_nsec > t.ts.tv_nsec)
	{
		return true;
	}
	else if (t.ts.tv_nsec > this->ts.tv_nsec)
	{
		return false;
	}
	return false;
}

bool Time::operator<(Time t)
{
	if (this->ts.tv_sec < t.ts.tv_sec)
	{
		return true;
	}
	else if (t.ts.tv_sec < this->ts.tv_sec)
	{
		return false;
	}
	else if (this->ts.tv_nsec < t.ts.tv_nsec)
	{
		return true;
	}
	else if (t.ts.tv_nsec < this->ts.tv_nsec)
	{
		return false;
	}
	return false;
}

Duration Time::operator-(Time t)
{
	Assert(this->ts.tv_sec > t.ts.tv_sec || (this->ts.tv_sec == t.ts.tv_sec && this->ts.tv_nsec >= t.ts.tv_nsec));
	const auto NanosecondsPerSecond = 1000000000LL;
	return
	{
		.nanoseconds = ((this->ts.tv_sec - t.ts.tv_sec) * NanosecondsPerSecond) + (this->ts.tv_nsec - t.ts.tv_nsec),
	};
}

Date Time::Date()
{
	auto tm = localtime(&this->ts.tv_sec);
	return
	{
		tm->tm_year,
		tm->tm_mon,
		tm->tm_mday,
		tm->tm_hour,
		tm->tm_min,
		tm->tm_sec,
		this->ts.tv_nsec,
	};
}

s64 Time::Year()
{
	auto tm = localtime(&this->ts.tv_sec);
	return tm->tm_year;
}

s64 Time::Month()
{
	auto tm = localtime(&this->ts.tv_sec);
	return tm->tm_mon;
}

s64 Time::Day()
{
	auto tm = localtime(&this->ts.tv_sec);
	return tm->tm_mday;
}

s64 Time::Hour()
{
	auto tm = localtime(&this->ts.tv_sec);
	return tm->tm_hour;
}

s64 Time::Minute()
{
	auto tm = localtime(&this->ts.tv_sec);
	return tm->tm_min;
}

s64 Time::Second()
{
	auto tm = localtime(&this->ts.tv_sec);
	return tm->tm_sec;
}

s64 Time::Nanosecond()
{
	return this->ts.tv_nsec;
}

Time Now()
{
	auto t = Time{};
	clock_gettime(CLOCK_MONOTONIC_RAW, &t.ts);
	return t;
}

void Sleep(s64 ns)
{
	auto s = s64(ns / Second);
	auto ts = (struct timespec)
	{
		.tv_sec = s,
		.tv_nsec = ns - (s * Second),
	};
	if (nanosleep(&ts, NULL))
	{
		log::Error("Time", "nanosleep() ended early: %k.\n", log::OSError());
	}
}

}
