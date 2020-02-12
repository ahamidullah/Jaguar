#pragma once

#if defined(__linux__)
	#include "Platform/Linux/Vulkan.h"
#else
	#error unsupported platform
#endif

#if defined(USE_VULKAN_RENDER_API)

const char *PlatformGetRequiredVulkanSurfaceInstanceExtension();
void PlatformCreateVulkanSurface(VkInstance instance, VkSurfaceKHR *surface);

#endif
