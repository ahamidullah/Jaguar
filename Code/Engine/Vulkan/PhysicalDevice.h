#pragma once

#ifdef VulkanBuild

#include "Queue.h"
#include "Media/Window.h"

namespace GPU::Vulkan
{

struct Instance;

struct PhysicalDevice
{
	VkPhysicalDevice physicalDevice;
	array::Static<s64, (s64)QueueType::Count> queueFamilies;
	VkSurfaceKHR surface;
	VkSurfaceFormatKHR surfaceFormat;
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	VkPresentModeKHR presentMode;
	VkPhysicalDeviceMemoryProperties memoryProperties;
};

PhysicalDevice NewPhysicalDevice(Instance inst, Window *w);

}

#endif
