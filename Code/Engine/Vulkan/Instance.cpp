#pragma once

#ifdef VulkanBuild

#include "PhysicalDevice.h"

namespace GPU::Vulkan
{

u32 DebugMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT sev, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT *data, void *userData)
{
	auto log = LogLevel{};
	auto sevStr = String{};
	switch (sev)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
	{
		log = VerboseLog;
		sevStr = "Verbose";
	} break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
	{
		log = InfoLog;
		sevStr = "Info";
	} break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
	{
		log = ErrorLog;
		sevStr = "Warning";
	} break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
	{
		log = ErrorLog;
		sevStr = "Error";
	} break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
	default:
	{
		log = ErrorLog;
		sevStr = "Unknown";
	};
	}
	auto typeStr = String{};
	switch (type)
	{
	case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
	{
		typeStr = "General";
	} break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
	{
		typeStr = "Validation";
	} break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
	{
		typeStr = "Performance";
	} break;
	default:
	{
		typeStr = "Unknown";
	};
	}
	if (sevStr == "Error" && typeStr == "Validation")
	{
		Abort("Vulkan", "%k: %k: %s", sevStr, typeStr, data->pMessage);
	}
	LogPrint(log, "Vulkan", "%k: %k: %s", sevStr, typeStr, data->pMessage);
    return 0;
}

Instance NewInstance(array::View<const char *> instLayers, array::View<const char *> instExts)
{
	auto nInstLayers = u32{};
	vkEnumerateInstanceLayerProperties(&nInstLayers, NULL);
	auto allInstLayers = array::New<VkLayerProperties>(nInstLayers);
	vkEnumerateInstanceLayerProperties(&nInstLayers, &allInstLayers[0]);
	LogVerbose("Vulkan", "Available Vulkan instance layers:");
	for (auto l : allInstLayers)
	{
		LogVerbose("Vulkan", "\t%s", l.layerName);
	}
	auto nInstExts = u32{0};
	Check(vkEnumerateInstanceExtensionProperties(NULL, &nInstExts, NULL));
	auto allInstExts = array::New<VkExtensionProperties>(nInstExts);
	Check(vkEnumerateInstanceExtensionProperties(NULL, &nInstExts, allInstExts.elements));
	LogVerbose("Vulkan", "Available Vulkan instance extensions:");
	for (auto e : allInstExts)
	{
		LogVerbose("Vulkan", "\t%s", e.extensionName);
	}
	LogVerbose("Vulkan", "Enabled Vulkan instance layers:");
	for (auto l : instLayers)
	{
		LogVerbose("Vulkan", "\t%s", l);
	}
	LogVerbose("Vulkan", "Enabled Vulkan instance extensions:");
	for (auto e : instExts)
	{
		LogVerbose("Vulkan", "\t%s", e);
	}
	auto ai = VkApplicationInfo
	{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "Jaguar",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "Jaguar",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_MAKE_VERSION(1, 2, 0),
	};
	// @TODO: Check that all required extensions/layers are available.
	auto ci = VkInstanceCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &ai,
		.enabledLayerCount = u32(instLayers.count),
		.ppEnabledLayerNames = instLayers.elements,
		.enabledExtensionCount = u32(instExts.count),
		.ppEnabledExtensionNames = instExts.elements,
	};
	auto dbgInfo = VkDebugUtilsMessengerCreateInfoEXT
	{
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = DebugMessageCallback,
	};
	if (DebugBuild)
	{
		ci.pNext = &dbgInfo;
	}
	auto i = Instance{};
	Check(vkCreateInstance(&ci, NULL, &i.instance));
	return i;
}

}

#endif
