#pragma once

#ifdef VulkanBuild

namespace GPU::Vulkan
{

struct Instance
{
	VkInstance instance;
};

Instance NewInstance(ArrayView<const char *> instLayers, ArrayView<const char *> instExts);

}

#endif
