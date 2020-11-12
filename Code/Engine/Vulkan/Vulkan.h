#pragma once

#ifdef VulkanBuild

namespace GPU::Vulkan
{

#define VulkanExportedFunction(name) PFN_##name name = NULL;
#define VulkanGlobalFunction(name) PFN_##name name = NULL;
#define VulkanInstanceFunction(name) PFN_##name name = NULL;
#define VulkanDeviceFunction(name) PFN_##name name = NULL;
	#include "FunctionDefinitions.h"
#undef VulkanExportedFunction
#undef VulkanGlobalFunction
#undef VulkanInstanceFunction
#undef VulkanDeviceFunction

#define Check(x)\
	do { \
		auto _r = (x); \
		if (_r != VK_SUCCESS) Abort("Vulkan", "Check failed on %s: %k", #x, VkResultToString(_r)); \
	} while (0)

string::String VkResultToString(VkResult r);

}

#endif
