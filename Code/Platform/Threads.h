#pragma once

#if defined(__linux__)
	#include "Platform/Linux/Threads.h"
#else
	#error unsupported platform
#endif
