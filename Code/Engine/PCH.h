#pragma once

#include "Code/Media/PCH.h"

#include "Code/Basic/PCH.h"

#if defined(USING_VULKAN_API)
	// Vulkan
	#define VK_NO_PROTOTYPES
	#define VK_USE_PLATFORM_XLIB_KHR
	#include "vulkan/vulkan.h"
#endif

// Asset
#include "tiny_gltf.h"

// Math
#include <math.h>
#include <float.h>
