#pragma once

#ifdef VulkanBuild

#include "PhysicalDevice.h"

#ifdef __linux__
	xcb_connection_t *XCBConnection();

	namespace GPU::Vulkan
	{

	VkResult NewSurface(Window *w, VkInstance inst, VkSurfaceKHR *s)
	{
		auto ci = VkXcbSurfaceCreateInfoKHR
		{
			.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
			.connection = XCBConnection(),
			.window = w->xcbWindow,
		};
		return vkCreateXcbSurfaceKHR(inst, &ci, NULL, s);
	}

	}
#endif

namespace GPU::Vulkan
{

PhysicalDevice NewPhysicalDevice(Instance inst, Window *w)
{
	auto pd = PhysicalDevice{};
	Check(NewSurface(w, inst.instance, &pd.surface));
	auto foundSuitablePhysDev = false;
	auto nPhysDevs = u32{};
	Check(vkEnumeratePhysicalDevices(inst.instance, &nPhysDevs, NULL));
	if (nPhysDevs == 0)
	{
		Abort("Vulkan", "Could not find any graphics devices.");
	}
	LogVerbose("Vulkan", "Available graphics device count: %d.", nPhysDevs);
	auto physDevs = array::New<VkPhysicalDevice>(nPhysDevs);
	Check(vkEnumeratePhysicalDevices(inst.instance, &nPhysDevs, physDevs.elements));
	if (DebugBuild)
	{
		LogVerbose("Vulkan", "Available graphics devices:");
		for (auto pd : physDevs)
		{
			auto dp = VkPhysicalDeviceProperties{};
			vkGetPhysicalDeviceProperties(pd, &dp);
			LogVerbose("Vulkan", "\t%s", dp.deviceName);
		}
	}
	for (auto i = 0; i < physDevs.count; i += 1)
	{
		pd.physicalDevice = physDevs[i];
		if (DebugBuild)
		{
			auto dp = VkPhysicalDeviceProperties{};
			vkGetPhysicalDeviceProperties(pd.physicalDevice, &dp);
			LogVerbose("Vulkan", "Considering graphics device %s.", dp.deviceName);
		}
		auto df12 = VkPhysicalDeviceVulkan12Features
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
		};
		auto df = VkPhysicalDeviceFeatures2
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
			.pNext = &df12,
		};
		vkGetPhysicalDeviceFeatures2(pd.physicalDevice, &df);
		if (!df.features.samplerAnisotropy)
		{
			LogVerbose("Vulkan", "Skipping graphics device: missing VkPhysicalDeviceFeatures2.features.samplerAnisotropy.");
			continue;
		}
		if (!df.features.shaderSampledImageArrayDynamicIndexing)
		{
			LogVerbose("Vulkan", "Skipping graphics device: missing VkPhysicalDeviceFeatures2.features.shaderSampledImageArrayDynamicIndexing.");
			continue;
		}
		if (!df.features.multiDrawIndirect)
		{
			LogVerbose("Vulkan", "Skipping graphics device: missing VkPhysicalDeviceFeatures2.features.multiDrawIndirect.");
			continue;
		}
		if (!df.features.shaderInt64)
		{
			LogVerbose("Vulkan", "Skipping graphics device: missing VkPhysicalDeviceFeatures2.features.shaderInt64.");
			continue;
		}
		if (!df12.shaderUniformBufferArrayNonUniformIndexing)
		{
			LogVerbose("Vulkan", "Skipping graphics device: missing VkPhysicalDeviceVulkan12Features.shaderUniformBufferArrayNonUniformIndexing");
			continue;
		}
		if (!df12.runtimeDescriptorArray)
		{
			LogVerbose("Vulkan", "Skipping graphics device: missing VkPhysicalDeviceVulkan12Features.runtimeDescriptorArray");
			continue;
		}
		if (!df12.bufferDeviceAddress)
		{
			LogVerbose("Vulkan", "Skipping graphics device: missing VkPhysicalDeviceVulkan12Features.bufferDeviceAddress.");
			continue;
		}
		auto nDevExts = u32{};
		Check(vkEnumerateDeviceExtensionProperties(pd.physicalDevice, NULL, &nDevExts, NULL));
		auto devExts = array::New<VkExtensionProperties>(nDevExts);
		Check(vkEnumerateDeviceExtensionProperties(pd.physicalDevice, NULL, &nDevExts, devExts.elements));
		auto missingDevExt = (const char *){};
		auto reqDevExts = array::MakeStatic<const char *>(
			"VK_KHR_swapchain",
			"VK_EXT_memory_budget");
		for (auto rde : reqDevExts)
		{
			missingDevExt = rde;
			for (auto de : devExts)
			{
				if (CStringsEqual(de.extensionName, rde))
				{
					missingDevExt = NULL;
					break;
				}
			}
			if (missingDevExt)
			{
				break;
			}
		}
		if (missingDevExt)
		{
			LogVerbose("Vulkan", "Skipping graphics device: missing device extension %s.", missingDevExt);
			continue;
		}
		// Make sure the swap chain is compatible with our window surface.
		// If we have at least one supported surface format and present mode, we will consider the device.
		auto nSurfFmts = u32{};
		Check(vkGetPhysicalDeviceSurfaceFormatsKHR(pd.physicalDevice, pd.surface, &nSurfFmts, NULL));
		if (nSurfFmts == 0)
		{
			LogVerbose("Vulkan", "Skipping graphics device: no surface formats.");
			continue;
		}
		// Select the best swap chain settings.
		auto surfFmts = array::New<VkSurfaceFormatKHR>(nSurfFmts);
		Check(vkGetPhysicalDeviceSurfaceFormatsKHR(pd.physicalDevice, pd.surface, &nSurfFmts, &surfFmts[0]));
		pd.surfaceFormat = surfFmts[0];
		if (nSurfFmts == 1 && surfFmts[0].format == VK_FORMAT_UNDEFINED)
		{
			// No preferred format, so we get to pick our own.
			pd.surfaceFormat = VkSurfaceFormatKHR{VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
		}
		else
		{
			for (auto sf : surfFmts)
			{
				if (sf.format == VK_FORMAT_B8G8R8A8_UNORM && sf.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				{
					pd.surfaceFormat = sf;
					break;
				}
			}
		}
		// VK_PRESENT_MODE_IMMEDIATE_KHR is for applications that don't care about tearing, or have some way of synchronizing their
		// rendering with the display.
		//
		// VK_PRESENT_MODE_MAILBOX_KHR may be useful for applications that generally render a new presentable image every refresh cycle,
		// but are occasionally early.  In this case, the application wants the new image to be displayed instead of the
		// previously-queued-for-presentation image that has not yet been displayed.
		//
		// VK_PRESENT_MODE_FIFO_KHR specifies that the presentation engine waits for the next vertical blanking period to update the
		// current image. REQUIRED to be supported.  This is for applications that don't want tearing ever. It's difficult to say how fast
		// they may be, whether they care about stuttering/latency.
		//
		// VK_PRESENT_MODE_FIFO_RELAXED_KHR is for applications that generally render a new presentable image every refresh cycle, but are
		// occasionally late.  In this case (perhaps because of stuttering/latency concerns), the application wants the late image to be
		// immediately displayed, even though that may mean some tearing.
		auto nPresentModes = u32{};
		Check(vkGetPhysicalDeviceSurfacePresentModesKHR(pd.physicalDevice, pd.surface, &nPresentModes, NULL));
		if (nPresentModes == 0)
		{
			LogVerbose("Vulkan", "Skipping graphics device: no present modes.");
			continue;
		}
		pd.presentMode = VK_PRESENT_MODE_FIFO_KHR;
		auto presentModes = array::New<VkPresentModeKHR>(nPresentModes);
		Check(vkGetPhysicalDeviceSurfacePresentModesKHR(pd.physicalDevice, pd.surface, &nPresentModes, presentModes.elements));
		for (auto pm : presentModes)
		{
			// @TODO: If vsync...
			if (pm == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				pd.presentMode = pm;
				break;
			}
			// @TODO: If not vsync... first of VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR
		}
		auto nQueueFams = u32{};
		vkGetPhysicalDeviceQueueFamilyProperties(pd.physicalDevice, &nQueueFams, NULL);
		auto queueFams = array::New<VkQueueFamilyProperties>(nQueueFams);
		vkGetPhysicalDeviceQueueFamilyProperties(pd.physicalDevice, &nQueueFams, queueFams.elements);
		pd.queueFamilies[(s64)QueueType::Graphics] = -1;
		pd.queueFamilies[(s64)QueueType::Compute] = -1;
		pd.queueFamilies[(s64)QueueType::Present] = -1;
		for (auto i = 0; i < nQueueFams; i++)
		{
			if (queueFams[i].queueCount == 0)
			{
				continue;
			}
			if (queueFams[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				pd.queueFamilies[(s64)QueueType::Graphics] = i;
			}
			if (queueFams[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				pd.queueFamilies[(s64)QueueType::Compute] = i;
			}
			// The graphics queue family can always be used as the transfer queue family, but if we can find a dedicated transfer queue
			// family, use that instead.
			if ((queueFams[i].queueFlags & VK_QUEUE_TRANSFER_BIT) && !(queueFams[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
			{
				pd.queueFamilies[(s64)QueueType::Transfer] = i;
			}
			else if (pd.queueFamilies[(s64)QueueType::Transfer] == -1 && (queueFams[i].queueFlags & VK_QUEUE_TRANSFER_BIT))
			{
				pd.queueFamilies[(s64)QueueType::Transfer] = i;
			}
			auto surfSupported = u32{};
			Check(vkGetPhysicalDeviceSurfaceSupportKHR(pd.physicalDevice, i, pd.surface, &surfSupported));
			if (surfSupported)
			{
				pd.queueFamilies[(s64)QueueType::Present] = i;
			}
		}
		if (pd.queueFamilies[(s64)QueueType::Graphics] == -1)
		{
			LogVerbose("Vulkan", "Skipping graphics device: missing graphics queue family.");
			continue;
		}
		if (pd.queueFamilies[(s64)QueueType::Compute] == -1)
		{
			LogVerbose("Vulkan", "Skipping graphics device: missing compute queue family.");
			continue;
		}
		if (pd.queueFamilies[(s64)QueueType::Present] == -1)
		{
			LogVerbose("Vulkan", "Skipping graphics device: missing present queue family.");
			continue;
		}
		// Ok, the physical device is suitable!
		Check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pd.physicalDevice, pd.surface, &pd.surfaceCapabilities));
		vkGetPhysicalDeviceMemoryProperties(pd.physicalDevice, &pd.memoryProperties); // No return.
		foundSuitablePhysDev = true;
		LogVerbose("Vulkan", "Vulkan device extensions:");
		for (auto de : devExts)
		{
			LogVerbose("Vulkan", "\t%s", de.extensionName);
		}
		break;
	}
	if (!foundSuitablePhysDev)
	{
		Abort("Vulkan", "Could not find suitable graphics device.");
	}
	return pd;
}

}

#endif
