#pragma once

#ifdef __linux__
	#include "Fiber_Linux.h"
#else
	#error Unsupported platform.
#endif
