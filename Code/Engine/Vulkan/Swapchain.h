#pragma once

#ifdef VulkanBuild

namespace GPU
{

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
	u64 frameIndex;
	StaticArray<VkSemaphore, _MaxFramesInFlight> imageOwnershipSemaphores;
	StaticArray<VkSemaphore, _MaxFramesInFlight> imageAcquiredSemaphores;
	Array<VkCommandBuffer> changeImageOwnershipFromGraphicsToPresentQueueCommands;

	void Initialize();
	void AcquireNextImage();
	void Present();
};

extern Swapchain swapchain;

}

#endif
