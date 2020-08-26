#pragma once

#ifdef __linux__
	#include "Linux/CPU.h"
#else
	#error Unsupported platform.
#endif
