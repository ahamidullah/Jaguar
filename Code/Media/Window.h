#pragma once

#ifdef __linux__
	#include "Linux/Window.h"
#else
	#error Unsupported platform.
#endif
