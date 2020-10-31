#pragma once

#ifdef VulkanBuild

namespace GPU
{

struct PhysicalDevice
{
	VkPhysicalDevice physicalDevice;
	StaticArray<s64, (s64)QueueType::Count> queueFamilies;
	VkSurfaceKHR surface;
	VkSurfaceFormatKHR surfaceFormat;
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	VkPresentModeKHR presentMode;
	VkPhysicalDeviceMemoryProperties memoryProperties;

	void Initialize(Window *w);
};

extern PhysicalDevice physicalDevice;

}

#endif
