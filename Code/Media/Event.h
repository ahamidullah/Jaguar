#pragma once

#if __linux__
	#include "Linux/Event.h"
#else
	#error Unsupported platform.
#endif
