#ifdef VulkanBuild

namespace GPU::Vulkan
{

Device NewDevice(PhysicalDevice pd, array::View<const char *> instLayers, array::View<const char *> devExts)
{
	auto prio = 1.0f;
	auto qcis = array::Array<VkDeviceQueueCreateInfo>{};
	qcis.Append(
		{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = u32(pd.queueFamilies[s64(QueueType::Graphics)]),
			.queueCount = 1,
			.pQueuePriorities = &prio,
		});
	if (pd.queueFamilies[s64(QueueType::Present)] != pd.queueFamilies[s64(QueueType::Graphics)])
	{
		qcis.Append(
			{
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = u32(pd.queueFamilies[s64(QueueType::Present)]),
				.queueCount = 1,
				.pQueuePriorities = &prio,
			});
	}
	if (pd.queueFamilies[s64(QueueType::Transfer)] != pd.queueFamilies[s64(QueueType::Graphics)])
	{
		qcis.Append(
			{
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = u32(pd.queueFamilies[s64(QueueType::Transfer)]),
				.queueCount = 1,
				.pQueuePriorities = &prio,
			});
	}
	auto pdf = VkPhysicalDeviceFeatures
	{
		.multiDrawIndirect = true,
		.samplerAnisotropy = true,
		.shaderInt64 = true,
	};
	auto pdf12 = VkPhysicalDeviceVulkan12Features
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
		.shaderUniformBufferArrayNonUniformIndexing = true,
		.runtimeDescriptorArray = true,
		.bufferDeviceAddress = true,
	};
	auto dci = VkDeviceCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &pdf12,
		.queueCreateInfoCount = u32(qcis.count),
		.pQueueCreateInfos = qcis.elements,
		.enabledLayerCount = u32(instLayers.count),
		.ppEnabledLayerNames = instLayers.elements,
		.enabledExtensionCount = u32(devExts.count),
		.ppEnabledExtensionNames = devExts.elements,
		.pEnabledFeatures = &pdf,
	};
	auto d = Device{};
	Check(vkCreateDevice(pd.physicalDevice, &dci, NULL, &d.device));
	return d;
}

}

#endif
