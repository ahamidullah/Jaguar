#pragma once

#if defined(__linux__)
	#include "Linux/Process.h"
#else
	#error Unsupported platform.
#endif
