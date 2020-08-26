#pragma once

#ifdef __linux__
	#include "Linux/Atomic.h"
#else
	#error Unsupported platform.
#endif
