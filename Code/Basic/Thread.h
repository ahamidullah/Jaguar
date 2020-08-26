#pragma once

#if __linux__
	#include "Linux/Thread.h"
#else
	#error Unsupported platform.
#endif
