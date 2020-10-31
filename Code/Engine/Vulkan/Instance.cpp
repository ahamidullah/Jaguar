#pragma once

#ifdef VulkanBuild

#include "PhysicalDevice.h"

extern VkInstance vkInstance;

namespace GPU
{

#ifdef __linux__

const char *RequiredSurfaceInstanceExtension()
{
	return "VK_KHR_xcb_surface";
}

#endif

auto instance = VkInstance{};

void InitializeInstance()
{
	auto reqInstLayers = Array<const char *>{};
	if (DebugBuild)
	{
		reqInstLayers.Append("VK_LAYER_KHRONOS_validation");
	}
	auto reqInstExts = MakeArray<const char *>(
		RequiredSurfaceInstanceExtension(),
		"VK_KHR_surface",
		"VK_KHR_get_physical_device_properties2");
	if (DebugBuild)
	{
		reqInstExts.Append("VK_EXT_debug_utils");
	}
	auto nInstLayers = u32{};
	vkEnumerateInstanceLayerProperties(&nInstLayers, NULL);
	auto instLayers = NewArray<VkLayerProperties>(nInstLayers);
	vkEnumerateInstanceLayerProperties(&nInstLayers, &instLayers[0]);
	LogVerbose("Vulkan", "Vulkan layers:");
	for (auto l : instLayers)
	{
		LogVerbose("Vulkan", "\t%s", l.layerName);
	}
	auto nInstExts = u32{0};
	VkCheck(vkEnumerateInstanceExtensionProperties(NULL, &nInstExts, NULL));
	auto instExts = NewArray<VkExtensionProperties>(nInstExts);
	VkCheck(vkEnumerateInstanceExtensionProperties(NULL, &nInstExts, instExts.elements));
	LogVerbose("Vulkan", "Available Vulkan instance extensions:");
	for (auto e : instExts)
	{
		LogVerbose("Vulkan", "\t%s", e.extensionName);
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
		.enabledLayerCount = (u32)reqInstLayers.count,
		.ppEnabledLayerNames = reqInstLayers.elements,
		.enabledExtensionCount = (u32)reqInstExts.count,
		.ppEnabledExtensionNames = reqInstExts.elements,
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
		.pfnUserCallback = VulkanDebugMessageCallback,
	};
	if (DebugBuild)
	{
		ci.pNext = &dbgInfo;
	}
	VkCheck(vkCreateInstance(&ci, NULL, &instance));
}

VkInstance Instance()
{
	return vkInstance;
	//return instance;
}

}

#endif
