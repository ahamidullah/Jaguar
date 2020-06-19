#pragma once

#if defined(__linux__)
	#include "Linux/Time.h"
#else
	#error Unsupported platform.
#endif
