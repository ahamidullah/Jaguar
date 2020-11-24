#pragma once

#if defined(__linux__)
	#include "Linux/Fiber.h"
#else
	#error Unsupported platform.
#endif
