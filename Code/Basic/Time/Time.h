#pragma once

#if defined(__linux__)
	#include "TimeLinux.h"
#else
	#error Unsupported platform.
#endif

namespace time
{

const auto Nanosecond = 1LL;
const auto Millisecond = 1000000LL;
const auto Second = 1000000000LL;
const auto Minute = 60000000000LL;
const auto Hour = 1200000000000LL;

}
