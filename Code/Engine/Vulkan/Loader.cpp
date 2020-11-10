#ifdef VulkanBuild

#include "Loader.h"

namespace GPU::Vulkan
{

void LoadExportedFunctions(DLL d)
{
	auto err = false;
	#define VulkanExportedFunction(name) \
		name = (PFN_##name)d.LookupProcedure(#name, &err); \
		if (err) \
		{ \
			Abort("Vulkan", "Failed to load Vulkan exported function %s.", #name); \
		}
	#include "FunctionDefinitions.h"
	#undef VulkanExportedFunction
}

void LoadGlobalFunctions()
{
	#define VulkanGlobalFunction(name) \
		name = (PFN_##name)vkGetInstanceProcAddr(NULL, (const char *)#name); \
		if (!name) \
		{ \
			Abort("Vulkan", "Failed to load Vulkan global function %s.", #name); \
		}
	#include "FunctionDefinitions.h"
	#undef VulkanGlobalFunction
}

void LoadInstanceFunctions(Instance i)
{
	#define VulkanInstanceFunction(name) \
		name = (PFN_##name)vkGetInstanceProcAddr(i.instance, (const char *)#name); \
		if (!name) \
		{ \
			Abort("Vulkan", "Failed to load Vulkan instance function %s.", #name); \
		}
	#include "FunctionDefinitions.h"
	#undef VulkanInstanceFunction
}

void LoadDeviceFunctions(Device d)
{
	#define VulkanDeviceFunction(name) \
		name = (PFN_##name)vkGetDeviceProcAddr(d.device, (const char *)#name); \
		if (!name) \
		{ \
			Abort("Vulkan", "Failed to load Vulkan instance function %s.", #name); \
		}
	#include "FunctionDefinitions.h"
	#undef VulkanDeviceFunction
}

}

#endif
