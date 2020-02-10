#if defined(USE_VULKAN_RENDER_API)

#include "Platform/Vulkan.h"

const char *PlatformGetRequiredVulkanSurfaceInstanceExtension() {
	return "VK_KHR_xlib_surface";
}

void PlatformCreateVulkanSurface(VkInstance instance, VkSurfaceKHR *surface) {
	VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
		.dpy = linuxContext.display,
		.window = linuxContext.window,
	};
	VK_CHECK(vkCreateXlibSurfaceKHR(instance, &surfaceCreateInfo, NULL, surface));
}

#endif
