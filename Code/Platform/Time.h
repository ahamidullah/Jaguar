#pragma once

#if defined(__linux__)
	#include "Platform/Linux/Time.h"
#else
	#error unsupported platform
#endif
