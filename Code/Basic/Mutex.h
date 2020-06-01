#pragma once

#if defined(__linux__)
	#include "Linux/Mutex.h"
#else
	#error Unsupported platform.
#endif
