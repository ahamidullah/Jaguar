#pragma once

#if defined(__linux__)
	#include "Platform/Linux/Memory.h"
#else
	#error unsupported platform
#endif
