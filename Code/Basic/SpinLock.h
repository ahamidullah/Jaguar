#pragma once

#if defined(__linux__)
	#include "Linux/SpinLock.h"
#else
	#error Unsupported platform.
#endif
