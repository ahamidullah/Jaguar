#pragma once

#if defined(__linux__)
	#include "Platform/Linux/Vulkan.h"
#else
	#error unsupported platform
#endif
