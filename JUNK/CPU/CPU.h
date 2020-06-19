#pragma once

#if defined(__linux__)
	#include "Linux/CPU.h"
#else
	#error Unsupported platform.
#endif
