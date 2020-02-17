#pragma once

#if defined(USE_VULKAN_RENDER_API)

const char *PlatformGetRequiredVulkanSurfaceInstanceExtension();
void PlatformCreateVulkanSurface(WindowHandle window, VkInstance instance, VkSurfaceKHR *surface);

#endif
