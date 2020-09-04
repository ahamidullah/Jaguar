#pragma once

#ifdef __linux__
	#include "Linux/Platform.h"
#else
	#error Unsupported platform.
#endif
