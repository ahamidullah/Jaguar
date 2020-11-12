#pragma once

#ifdef VulkanBuild

namespace GPU::Vulkan
{

struct PhysicalDevice;
struct Device;

const auto _MaxFramesInFlight = 2;

struct Swapchain
{
	VkSwapchainKHR swapchain;
	array::Array<VkImage> images;
	array::Array<VkImageView> imageViews;
	u32 imageIndex;
	VkImage defaultDepthImage;
	VkImageView defaultDepthImageView;
	array::Static<VkFence, _MaxFramesInFlight> frameFences;
	array::Static<VkSemaphore, _MaxFramesInFlight> imageOwnershipSemaphores;
	array::Static<VkSemaphore, _MaxFramesInFlight> imageAcquiredSemaphores;
	array::Array<VkCommandBuffer> changeImageOwnershipFromGraphicsToPresentQueueCommands;

	void AcquireNextImage(Device d, s64 frameIndex);
	void Present(PhysicalDevice pd, Queues q, s64 frameIndex);
};

Swapchain NewSwapchain(PhysicalDevice pd, Device d);

}

#endif
