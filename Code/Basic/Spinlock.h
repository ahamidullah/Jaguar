#pragma once

#if defined(__linux__)
	#include "Linux/Spinlock.h"
#else
	#error Unsupported platform.
#endif
