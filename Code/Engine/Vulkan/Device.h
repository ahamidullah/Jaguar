#pragma once

#ifdef VulkanBuild

namespace GPU::Vulkan
{

struct Device
{
	VkDevice device;
};

Device NewDevice(PhysicalDevice pd, ArrayView<const char *> instLayers, ArrayView<const char *> devExts);

}

#endif
