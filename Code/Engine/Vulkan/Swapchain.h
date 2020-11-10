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
	Array<VkImage> images;
	Array<VkImageView> imageViews;
	u32 imageIndex;
	VkImage defaultDepthImage;
	VkImageView defaultDepthImageView;
	StaticArray<VkFence, _MaxFramesInFlight> frameFences;
	StaticArray<VkSemaphore, _MaxFramesInFlight> imageOwnershipSemaphores;
	StaticArray<VkSemaphore, _MaxFramesInFlight> imageAcquiredSemaphores;
	Array<VkCommandBuffer> changeImageOwnershipFromGraphicsToPresentQueueCommands;

	void AcquireNextImage(Device d, s64 frameIndex);
	void Present(PhysicalDevice pd, Queues q, s64 frameIndex);
};

Swapchain NewSwapchain(PhysicalDevice pd, Device d);

}

#endif
