#ifdef VulkanBuild

// @TODO: Should this file exist.

#include "Swapchain.h"
#include "CommandBuffer.h"

namespace GPU
{

void BeginFrame()
{
	swapchain.AcquireNextImage();
	GPUBeginFrame(); // @TODO
	//commandBufferAllocator.FreeFrame(vkCommandGroupFreeIndex);
	//asyncCommandBufferAllocator.FreeFinished();
}

void EndFrame()
{
	swapchain.Present();
	GPUEndFrame(); // @TODO
}

}

#endif
