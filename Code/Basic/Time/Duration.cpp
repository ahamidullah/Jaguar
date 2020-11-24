#include "Duration.h"

namespace time
{

bool Duration::operator>(Duration d)
{
	return this->nanoseconds > d.nanoseconds;
}

bool Duration::operator<(Duration d)
{
	return this->nanoseconds < d.nanoseconds;
}

Duration Duration::operator-(Duration d)
{
	return
	{
		.nanoseconds = this->nanoseconds - d.nanoseconds,
	};
}

s64 Duration::Hours()
{
	return this->nanoseconds / Hour;
}

s64 Duration::Minutes()
{
	return this->nanoseconds / Minute;
}

s64 Duration::Seconds()
{
	return this->nanoseconds / Second;
}

s64 Duration::Milliseconds()
{
	return this->nanoseconds / Millisecond;
}

s64 Duration::Nanoseconds()
{
	return this->nanoseconds;
}

}
