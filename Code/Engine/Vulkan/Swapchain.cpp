#ifdef VulkanBuild

#include "PhysicalDevice.h"
#include "Queue.h"

namespace GPU
{

//auto swapchain = VkSwapchainKHR{};
//auto swapchainImages = Array<VkImage>{};
//auto swapchainImageViews = Array<VkImageView>{};
//auto swapchainImageIndex = u32{};
//auto swapchainSurface = VkSurfaceKHR{};
//auto swapchainDefaultFramebufferAttachments = Array<Array<VkImageView>>{};

auto swapchain = Swapchain{};

const auto DepthBufferFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
const auto DepthBufferInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
const auto DepthBufferImageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
const auto DepthBufferSampleCount = VK_SAMPLE_COUNT_1_BIT;

void Swapchain::Initialize()
{
	auto surfFmt = &physicalDevice.surfaceFormat;
	// Create swapchain.
	{
		auto surfCaps = &physicalDevice.surfaceCapabilities;
		auto ext = VkExtent2D{};
		if (surfCaps->currentExtent.width == U32Max && surfCaps->currentExtent.height == U32Max)
		{
			// Indicates Vulkan will accept any extent dimension.
			ext.width = RenderWidth();
			ext.height = RenderHeight();
		}
		else
		{
			ext.width = Maximum(surfCaps->minImageExtent.width, Minimum(surfCaps->maxImageExtent.width, RenderWidth()));
			ext.height = Maximum(surfCaps->minImageExtent.height, Minimum(surfCaps->maxImageExtent.height, RenderHeight()));
		}
		// @TODO: Why is this a problem?
		if (ext.width != RenderWidth() && ext.height != RenderHeight())
		{
			Abort("Vulkan", "Swapchain image dimensions do not match the window dimensions: swapchain %ux%u, window %ux%u.", ext.width, ext.height, RenderWidth(), RenderHeight());
		}
		auto minImgs = surfCaps->minImageCount + 1;
		if (surfCaps->maxImageCount > 0 && (minImgs > surfCaps->maxImageCount))
		{
			minImgs = surfCaps->maxImageCount;
		}
		auto ci = VkSwapchainCreateInfoKHR
		{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = physicalDevice.surface,
			.minImageCount = minImgs,
			.imageFormat = surfFmt->format,
			.imageColorSpace  = surfFmt->colorSpace,
			.imageExtent = ext,
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.preTransform = surfCaps->currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = physicalDevice.presentMode,
			.clipped = 1,
			.oldSwapchain = NULL,
		};
		if (physicalDevice.queueFamilies[(s64)QueueType::Present] != physicalDevice.queueFamilies[(s64)QueueType::Graphics])
		{
			auto fams = MakeStaticArray(
				(u32)physicalDevice.queueFamilies[(s64)QueueType::Graphics],
				(u32)physicalDevice.queueFamilies[(s64)QueueType::Present]);
			ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			ci.queueFamilyIndexCount = fams.Count();
			ci.pQueueFamilyIndices = fams.elements;
		}
		else
		{
			ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}
		VkCheck(vkCreateSwapchainKHR(vkDevice, &ci, NULL, &this->swapchain));
	}
	// Get swapchain images.
	{
		auto n = u32{};
		VkCheck(vkGetSwapchainImagesKHR(vkDevice, this->swapchain, &n, NULL));
		this->images.Resize(n);
		VkCheck(vkGetSwapchainImagesKHR(vkDevice, this->swapchain, &n, &this->images[0]));
		this->imageViews.Resize(n);
		for (auto i = 0; i < n; i++)
		{
			auto cm = VkComponentMapping 
			{
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY,
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY,
			};
			auto isr = VkImageSubresourceRange 
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			};
			auto ci = VkImageViewCreateInfo
			{
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image = this->images[i],
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.format = surfFmt->format,
				.components = cm,
				.subresourceRange = isr,
			};
			VkCheck(vkCreateImageView(vkDevice, &ci, NULL, &this->imageViews[i]));
		}
	}
	// Create default depth image.
	{
		auto ici = VkImageCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = DepthBufferFormat,
			.extent =
			{
				.width = (u32)RenderWidth(),
				.height = (u32)RenderHeight(),
				.depth = 1,
			},
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = DepthBufferSampleCount,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = DepthBufferImageUsage,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = DepthBufferInitialLayout,
		};
		VkCheck(vkCreateImage(vkDevice, &ici, NULL, &this->defaultDepthImage));
		auto mr = VkMemoryRequirements{};
		vkGetImageMemoryRequirements(vkDevice, this->defaultDepthImage, &mr);
		// @TODO: Use the image allocator.
		auto ai = VkMemoryAllocateInfo
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = mr.size,
			.memoryTypeIndex = vkMemoryTypeToMemoryIndex[VulkanGPUMemory], // @TODO
		};
		auto mem = VkDeviceMemory{};
		VkCheck(vkAllocateMemory(vkDevice, &ai, NULL, &mem));
		VkCheck(vkBindImageMemory(vkDevice, this->defaultDepthImage, mem, 0));
		auto vci = VkImageViewCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = this->defaultDepthImage,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = DepthBufferFormat,
			.components = 
			{
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY,
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY,
			},
			.subresourceRange =
			{
				.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};
		VkCheck(vkCreateImageView(vkDevice, &vci, NULL, &this->defaultDepthImageView));
	}
	// Create frame fences.
	{
		auto ci = VkFenceCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT,
		};
		for (auto i = 0; i < _MaxFramesInFlight; i += 1)
		{
			VkCheck(vkCreateFence(vkDevice, &ci, NULL, &this->frameFences[i]));
		}
	}
	// Image semaphores.
	{
		auto ci = VkSemaphoreCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		};
		if (physicalDevice.queueFamilies[(s64)QueueType::Present] != physicalDevice.queueFamilies[(s64)QueueType::Graphics])
		{
			for (auto i = 0; i < _MaxFramesInFlight; i++)
			{
				VkCheck(vkCreateSemaphore(vkDevice, &ci, NULL, &this->imageOwnershipSemaphores[i]));
			}
		}
		for (auto i = 0; i < _MaxFramesInFlight; i++)
		{
			VkCheck(vkCreateSemaphore(vkDevice, &ci, NULL, &this->imageAcquiredSemaphores[i]));
			//VkCheck(vkCreateSemaphore(vkDevice, &ci, NULL, &vkFrameGraphicsCompleteSemaphores[i]));
			//VkCheck(vkCreateSemaphore(vkDevice, &ci, NULL, &vkFrameTransfersCompleteSemaphores[i]));
		}
	}
	// Create image ownership transition commands.
	{
		this->changeImageOwnershipFromGraphicsToPresentQueueCommands.Resize(this->images.count);
		auto ai = VkCommandBufferAllocateInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = vkPresentCommandPool, // @TODO
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = (u32)this->images.count,
		};
		VkCheck(vkAllocateCommandBuffers(vkDevice, &ai, this->changeImageOwnershipFromGraphicsToPresentQueueCommands.elements));
		for (auto i = 0; i < this->images.count; i += 1)
		{
			auto bi = VkCommandBufferBeginInfo
			{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
			};
			VkCheck(vkBeginCommandBuffer(this->changeImageOwnershipFromGraphicsToPresentQueueCommands[i], &bi));
			auto mb = VkImageMemoryBarrier
			{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				.srcQueueFamilyIndex = (u32)physicalDevice.queueFamilies[(s64)QueueType::Graphics],
				.dstQueueFamilyIndex = (u32)physicalDevice.queueFamilies[(s64)QueueType::Present],
				.image = this->images[i],
				.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
			};
			vkCmdPipelineBarrier(this->changeImageOwnershipFromGraphicsToPresentQueueCommands[i], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, NULL, 0, NULL, 1, &mb);
			VkCheck(vkEndCommandBuffer(this->changeImageOwnershipFromGraphicsToPresentQueueCommands[i]));
		}
	}
}

void Swapchain::AcquireNextImage()
{
	VkCheck(vkWaitForFences(vkDevice, 1, &this->frameFences[this->frameIndex], true, U32Max));
	VkCheck(vkResetFences(vkDevice, 1, &this->frameFences[this->frameIndex]));
	const auto AcquireImageTimeout = 2 * TimeSecond;
	VkCheck(vkAcquireNextImageKHR(vkDevice, this->swapchain, AcquireImageTimeout, this->imageAcquiredSemaphores[this->frameIndex], VK_NULL_HANDLE, &this->imageIndex));
}

void Swapchain::Present()
{
	if (physicalDevice.queueFamilies[(s64)QueueType::Present] != physicalDevice.queueFamilies[(s64)QueueType::Graphics])
	{
		// If we are using separate queues, change image ownership to the present queue before presenting, waiting for the draw complete
		// semaphore and signalling the ownership released semaphore when finished.
		auto stage = VkPipelineStageFlags{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		auto si = VkSubmitInfo
		{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &queues[(s64)QueueType::Graphics].frameSemaphores[this->frameIndex],
			.pWaitDstStageMask = &stage,
			.commandBufferCount = 1,
			.pCommandBuffers = &this->changeImageOwnershipFromGraphicsToPresentQueueCommands[this->imageIndex],
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &this->imageOwnershipSemaphores[this->frameIndex],
		};
		//VkCheck(vkQueueSubmit(GetDevice()->presentQueue, 1, &si, VK_NULL_HANDLE));
		VkCheck(vkQueueSubmit(vkQueues[VulkanPresentQueue], 1, &si, VK_NULL_HANDLE)); // @TODO
	}
	// If we are using separate queues we have to wait for image ownership, otherwise wait for draw complete.
	auto pi = VkPresentInfoKHR
	{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = NULL,
		.waitSemaphoreCount = 1,
		.swapchainCount = 1,
		.pSwapchains = &this->swapchain,
		.pImageIndices = &this->imageIndex,
	};
	if (physicalDevice.queueFamilies[(s64)QueueType::Present] != physicalDevice.queueFamilies[(s64)QueueType::Graphics])
	{
		pi.pWaitSemaphores = &this->imageOwnershipSemaphores[this->frameIndex];
	}
	else
	{
		pi.pWaitSemaphores = &queues[(s64)QueueType::Graphics].frameSemaphores[this->frameIndex];
	}
	VkCheck(vkQueuePresentKHR(vkQueues[VulkanPresentQueue], &pi)); // @TODO
	this->frameIndex = (this->frameIndex + 1) % MaxFramesInFlight;
}

}

#endif
