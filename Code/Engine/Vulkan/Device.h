#pragma once

#ifdef VulkanBuild

namespace GPU::Vulkan
{

struct Device
{
	VkDevice device;
};

Device NewDevice(PhysicalDevice pd, array::View<const char *> instLayers, array::View<const char *> devExts);

}

#endif
