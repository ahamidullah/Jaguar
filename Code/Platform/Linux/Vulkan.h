#pragma once

#if defined(USE_VULKAN_RENDER_API)

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_xlib.h> 

const char *PlatformGetRequiredVulkanSurfaceInstanceExtension();
void PlatformCreateVulkanSurface(VkInstance instance, VkSurfaceKHR *surface);

#endif
