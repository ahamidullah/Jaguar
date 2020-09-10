#pragma once

#include "Media/PCH.h"
#include "Basic/PCH.h"

#ifdef VulkanBuild
	#define VK_NO_PROTOTYPES
	#define VK_USE_PLATFORM_XCB_KHR
	#include "vulkan/vulkan.h"
#endif

// Asset
#include "tiny_gltf.h"

// Math
#include <math.h>
#include <float.h>
