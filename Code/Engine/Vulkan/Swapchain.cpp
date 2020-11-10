#ifdef VulkanBuild

#include "PhysicalDevice.h"
#include "Queue.h"
#include "Basic/Time/Time.h"

namespace GPU::Vulkan
{

//auto swapchain = VkSwapchainKHR{};
//auto swapchainImages = Array<VkImage>{};
//auto swapchainImageViews = Array<VkImageView>{};
//auto swapchainImageIndex = u32{};
//auto swapchainSurface = VkSurfaceKHR{};
//auto swapchainDefaultFramebufferAttachments = Array<Array<VkImageView>>{};

const auto DepthBufferFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
const auto DepthBufferInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
const auto DepthBufferImageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
const auto DepthBufferSampleCount = VK_SAMPLE_COUNT_1_BIT;

Swapchain NewSwapchain(PhysicalDevice pd, Device d)
{
	auto sc = Swapchain{};
	// Create swapchain.
	{
		auto ext = VkExtent2D{};
		if (pd.surfaceCapabilities.currentExtent.width == U32Max && pd.surfaceCapabilities.currentExtent.height == U32Max)
		{
			// Indicates Vulkan will accept any extent dimension.
			ext.width = RenderWidth();
			ext.height = RenderHeight();
		}
		else
		{
			ext.width = Maximum(pd.surfaceCapabilities.minImageExtent.width, Minimum(pd.surfaceCapabilities.maxImageExtent.width, RenderWidth()));
			ext.height = Maximum(pd.surfaceCapabilities.minImageExtent.height, Minimum(pd.surfaceCapabilities.maxImageExtent.height, RenderHeight()));
		}
		// @TODO: Why is this a problem?
		if (ext.width != RenderWidth() && ext.height != RenderHeight())
		{
			Abort("Vulkan", "Swapchain image dimensions do not match the window dimensions: swapchain %ux%u, window %ux%u.", ext.width, ext.height, RenderWidth(), RenderHeight());
		}
		auto minImgs = pd.surfaceCapabilities.minImageCount + 1;
		if (pd.surfaceCapabilities.maxImageCount > 0 && (minImgs > pd.surfaceCapabilities.maxImageCount))
		{
			minImgs = pd.surfaceCapabilities.maxImageCount;
		}
		auto ci = VkSwapchainCreateInfoKHR
		{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = pd.surface,
			.minImageCount = minImgs,
			.imageFormat = pd.surfaceFormat.format,
			.imageColorSpace  = pd.surfaceFormat.colorSpace,
			.imageExtent = ext,
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.preTransform = pd.surfaceCapabilities.currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = pd.presentMode,
			.clipped = 1,
			.oldSwapchain = NULL,
		};
		if (pd.queueFamilies[(s64)QueueType::Present] != pd.queueFamilies[(s64)QueueType::Graphics])
		{
			auto fams = MakeStaticArray(
				u32(pd.queueFamilies[s64(QueueType::Graphics)]),
				u32(pd.queueFamilies[s64(QueueType::Present)]));
			ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			ci.queueFamilyIndexCount = fams.Count();
			ci.pQueueFamilyIndices = fams.elements;
		}
		else
		{
			ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}
		VkCheck(vkCreateSwapchainKHR(d.device, &ci, NULL, &sc.swapchain));
	}
	// Get swapchain images.
	{
		auto n = u32{};
		VkCheck(vkGetSwapchainImagesKHR(d.device, sc.swapchain, &n, NULL));
		sc.images.Resize(n);
		VkCheck(vkGetSwapchainImagesKHR(d.device, sc.swapchain, &n, &sc.images[0]));
		sc.imageViews.Resize(n);
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
				.image = sc.images[i],
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.format = pd.surfaceFormat.format,
				.components = cm,
				.subresourceRange = isr,
			};
			VkCheck(vkCreateImageView(d.device, &ci, NULL, &sc.imageViews[i]));
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
		VkCheck(vkCreateImage(d.device, &ici, NULL, &sc.defaultDepthImage));
		auto mr = VkMemoryRequirements{};
		vkGetImageMemoryRequirements(d.device, sc.defaultDepthImage, &mr);
		// @TODO: Use the image allocator.
		auto ai = VkMemoryAllocateInfo
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = mr.size,
			.memoryTypeIndex = MemoryHeapIndex(pd.memoryProperties, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
		};
		auto mem = VkDeviceMemory{};
		VkCheck(vkAllocateMemory(d.device, &ai, NULL, &mem));
		VkCheck(vkBindImageMemory(d.device, sc.defaultDepthImage, mem, 0));
		auto vci = VkImageViewCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = sc.defaultDepthImage,
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
		VkCheck(vkCreateImageView(d.device, &vci, NULL, &sc.defaultDepthImageView));
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
			VkCheck(vkCreateFence(d.device, &ci, NULL, &sc.frameFences[i]));
		}
	}
	// Image semaphores.
	{
		auto ci = VkSemaphoreCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		};
		if (pd.queueFamilies[(s64)QueueType::Present] != pd.queueFamilies[(s64)QueueType::Graphics])
		{
			for (auto i = 0; i < _MaxFramesInFlight; i++)
			{
				VkCheck(vkCreateSemaphore(d.device, &ci, NULL, &sc.imageOwnershipSemaphores[i]));
			}
		}
		for (auto i = 0; i < _MaxFramesInFlight; i++)
		{
			VkCheck(vkCreateSemaphore(d.device, &ci, NULL, &sc.imageAcquiredSemaphores[i]));
		}
	}
	// Create image ownership transition commands.
	{
/*		@TODO: Allocate this out of the async queues.
		sc.changeImageOwnershipFromGraphicsToPresentQueueCommands.Resize(sc.images.count);
		auto ai = VkCommandBufferAllocateInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = q.commandPools, // @TODO
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = (u32)sc.images.count,
		};
		VkCheck(vkAllocateCommandBuffers(d.device, &ai, sc.changeImageOwnershipFromGraphicsToPresentQueueCommands.elements));
		for (auto i = 0; i < sc.images.count; i += 1)
		{
			auto bi = VkCommandBufferBeginInfo
			{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
			};
			VkCheck(vkBeginCommandBuffer(sc.changeImageOwnershipFromGraphicsToPresentQueueCommands[i], &bi));
			auto mb = VkImageMemoryBarrier
			{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				.srcQueueFamilyIndex = u32(pd.queueFamilies[(s64)QueueType::Graphics]),
				.dstQueueFamilyIndex = u32(pd.queueFamilies[(s64)QueueType::Present]),
				.image = sc.images[i],
				.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
			};
			vkCmdPipelineBarrier(sc.changeImageOwnershipFromGraphicsToPresentQueueCommands[i], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, NULL, 0, NULL, 1, &mb);
			VkCheck(vkEndCommandBuffer(sc.changeImageOwnershipFromGraphicsToPresentQueueCommands[i]));
		}
*/
	}
	return sc;
}

void Swapchain::AcquireNextImage(Device d, s64 frameIndex)
{
	VkCheck(vkWaitForFences(d.device, 1, &this->frameFences[frameIndex], true, U32Max));
	VkCheck(vkResetFences(d.device, 1, &this->frameFences[frameIndex]));
	const auto AcquireImageTimeout = 2 * Time::Second;
	VkCheck(vkAcquireNextImageKHR(d.device, this->swapchain, AcquireImageTimeout, this->imageAcquiredSemaphores[frameIndex], VK_NULL_HANDLE, &this->imageIndex));
}

void Swapchain::Present(PhysicalDevice pd, Queues q, s64 frameIndex)
{
	if (pd.queueFamilies[s64(QueueType::Present)] != pd.queueFamilies[s64(QueueType::Graphics)])
	{
		// If we are using separate queues, change image ownership to the present queue before presenting, waiting for the draw complete
		// semaphore and signalling the ownership released semaphore when finished.
		auto stage = VkPipelineStageFlags{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		auto si = VkSubmitInfo
		{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &q.submissionSemaphores[frameIndex][s64(QueueType::Graphics)],
			.pWaitDstStageMask = &stage,
			.commandBufferCount = 1,
			.pCommandBuffers = &this->changeImageOwnershipFromGraphicsToPresentQueueCommands[this->imageIndex],
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &this->imageOwnershipSemaphores[frameIndex],
		};
		VkCheck(vkQueueSubmit(q.queues[s64(QueueType::Present)], 1, &si, VK_NULL_HANDLE));
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
	if (pd.queueFamilies[s64(QueueType::Present)] != pd.queueFamilies[s64(QueueType::Graphics)])
	{
		pi.pWaitSemaphores = &this->imageOwnershipSemaphores[frameIndex];
	}
	else
	{
		pi.pWaitSemaphores = &q.submissionSemaphores[frameIndex][s64(QueueType::Graphics)];
	}
	VkCheck(vkQueuePresentKHR(q.queues[s64(QueueType::Present)], &pi));
}

}

#endif
