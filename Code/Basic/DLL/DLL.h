#pragma once

#if defined(__linux__)
	#include "Linux/DLL.h"
#else
	#error Unsupported platform.
#endif
