#pragma once

#if __linux__
	#include "Linux/Semaphore.h"
#else
	#error Unsupported platform.
#endif
