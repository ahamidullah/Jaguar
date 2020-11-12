#pragma once

#ifdef VulkanBuild

namespace GPU::Vulkan
{

struct Instance
{
	VkInstance instance;
};

Instance NewInstance(array::View<const char *> instLayers, array::View<const char *> instExts);

}

#endif
