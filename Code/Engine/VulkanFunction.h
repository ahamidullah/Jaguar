VK_EXPORTED_FUNCTION(vkGetInstanceProcAddr)

VK_GLOBAL_FUNCTION(vkCreateInstance)
VK_GLOBAL_FUNCTION(vkEnumerateInstanceVersion)
VK_GLOBAL_FUNCTION(vkEnumerateInstanceExtensionProperties)
VK_GLOBAL_FUNCTION(vkEnumerateInstanceLayerProperties)

VK_INSTANCE_FUNCTION(vkGetDeviceProcAddr)
VK_INSTANCE_FUNCTION(vkDestroyInstance)
VK_INSTANCE_FUNCTION(vkCreateDebugUtilsMessengerEXT)
VK_INSTANCE_FUNCTION(vkDestroyDebugUtilsMessengerEXT)
VK_INSTANCE_FUNCTION(vkEnumeratePhysicalDevices)
VK_INSTANCE_FUNCTION(vkCreateXcbSurfaceKHR)
VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceProperties)
VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceFeatures)
VK_INSTANCE_FUNCTION(vkEnumerateDeviceExtensionProperties)
VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceSurfaceFormatsKHR)
VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceSurfaceSupportKHR)
VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceSurfaceCapabilitiesKHR)
VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceSurfacePresentModesKHR)
VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties)
VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceMemoryProperties)
VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceMemoryProperties2)
VK_INSTANCE_FUNCTION(vkCreateDevice)
VK_INSTANCE_FUNCTION(vkDestroySurfaceKHR)
VK_INSTANCE_FUNCTION(vkDestroySwapchainKHR)

VK_DEVICE_FUNCTION(vkDeviceWaitIdle)
VK_DEVICE_FUNCTION(vkDestroyDevice)
VK_DEVICE_FUNCTION(vkGetDeviceQueue)
VK_DEVICE_FUNCTION(vkCreateSwapchainKHR)
VK_DEVICE_FUNCTION(vkGetSwapchainImagesKHR)
VK_DEVICE_FUNCTION(vkCreateImageView)
VK_DEVICE_FUNCTION(vkCreateRenderPass)
VK_DEVICE_FUNCTION(vkCreateShaderModule)
VK_DEVICE_FUNCTION(vkDestroyShaderModule)
VK_DEVICE_FUNCTION(vkCreatePipelineLayout)
VK_DEVICE_FUNCTION(vkCreateGraphicsPipelines)
VK_DEVICE_FUNCTION(vkCreatePipelineCache)
VK_DEVICE_FUNCTION(vkCreateFramebuffer)
VK_DEVICE_FUNCTION(vkCreateDescriptorSetLayout)
VK_DEVICE_FUNCTION(vkCreateDescriptorPool)
VK_DEVICE_FUNCTION(vkCreateCommandPool)
VK_DEVICE_FUNCTION(vkResetCommandPool)
VK_DEVICE_FUNCTION(vkDestroyCommandPool)
VK_DEVICE_FUNCTION(vkCreateBuffer)
VK_DEVICE_FUNCTION(vkCreateSampler)
VK_DEVICE_FUNCTION(vkDestroySampler)
VK_DEVICE_FUNCTION(vkAllocateCommandBuffers)
VK_DEVICE_FUNCTION(vkResetCommandBuffer)
VK_DEVICE_FUNCTION(vkAllocateDescriptorSets)
VK_DEVICE_FUNCTION(vkUpdateDescriptorSets)
VK_DEVICE_FUNCTION(vkBeginCommandBuffer)
VK_DEVICE_FUNCTION(vkCmdBeginRenderPass)
VK_DEVICE_FUNCTION(vkCmdBindPipeline)
VK_DEVICE_FUNCTION(vkCmdBindVertexBuffers)
VK_DEVICE_FUNCTION(vkCmdBindIndexBuffer)
VK_DEVICE_FUNCTION(vkCmdBindDescriptorSets)
VK_DEVICE_FUNCTION(vkCmdDraw)
VK_DEVICE_FUNCTION(vkCmdDrawIndexed)
VK_DEVICE_FUNCTION(vkCmdCopyBuffer)
VK_DEVICE_FUNCTION(vkCmdCopyBufferToImage)
VK_DEVICE_FUNCTION(vkCmdPipelineBarrier)
VK_DEVICE_FUNCTION(vkCmdSetViewport)
VK_DEVICE_FUNCTION(vkCmdSetScissor)
VK_DEVICE_FUNCTION(vkCmdSetDepthBias)
VK_DEVICE_FUNCTION(vkCmdPushConstants)
VK_DEVICE_FUNCTION(vkCmdEndRenderPass)
VK_DEVICE_FUNCTION(vkEndCommandBuffer)
VK_DEVICE_FUNCTION(vkCreateSemaphore)
VK_DEVICE_FUNCTION(vkDestroySemaphore)
VK_DEVICE_FUNCTION(vkAcquireNextImageKHR)
VK_DEVICE_FUNCTION(vkQueueSubmit)
VK_DEVICE_FUNCTION(vkQueueWaitIdle)
VK_DEVICE_FUNCTION(vkQueuePresentKHR)
VK_DEVICE_FUNCTION(vkCreateFence)
VK_DEVICE_FUNCTION(vkGetFenceStatus)
VK_DEVICE_FUNCTION(vkWaitForFences)
VK_DEVICE_FUNCTION(vkResetFences)
VK_DEVICE_FUNCTION(vkDestroyFence)
VK_DEVICE_FUNCTION(vkGetBufferMemoryRequirements)
VK_DEVICE_FUNCTION(vkAllocateMemory)
VK_DEVICE_FUNCTION(vkBindBufferMemory)
VK_DEVICE_FUNCTION(vkMapMemory)
VK_DEVICE_FUNCTION(vkUnmapMemory)
VK_DEVICE_FUNCTION(vkFreeMemory)
VK_DEVICE_FUNCTION(vkCreateImage)
VK_DEVICE_FUNCTION(vkDestroyImage)
VK_DEVICE_FUNCTION(vkGetImageMemoryRequirements)
VK_DEVICE_FUNCTION(vkBindImageMemory)
VK_DEVICE_FUNCTION(vkFreeCommandBuffers)
VK_DEVICE_FUNCTION(vkDestroyFramebuffer)
VK_DEVICE_FUNCTION(vkDestroyPipeline)
VK_DEVICE_FUNCTION(vkDestroyPipelineLayout)
VK_DEVICE_FUNCTION(vkDestroyRenderPass)
VK_DEVICE_FUNCTION(vkDestroyImageView)
VK_DEVICE_FUNCTION(vkDestroyBuffer)
VK_DEVICE_FUNCTION(vkDestroyDescriptorSetLayout)
VK_DEVICE_FUNCTION(vkDestroyDescriptorPool)
