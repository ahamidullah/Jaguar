#ifdef VulkanBuild

#include "Queue.h"

namespace GPU
{

auto queues = StaticArray<Queue, (s64)QueueType::Count>{};

void InitializeQueues()
{
	for (auto &q : queues)
	{
		auto ci = VkSemaphoreCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		};
		for (auto i = 0; i < _MaxFramesInFlight; i++)
		{
			VkCheck(vkCreateSemaphore(vkDevice, &ci, NULL, &q.frameSemaphores[i]));
		}
	}
}

void SubmitCommands(QueueType t, ArrayView<VkSemaphore> waitSems, ArrayView<VkPipelineStageFlags> waitStages, ArrayView<VkSemaphore> signalSems, VkFence f)
{
	auto cbs = Array<VkCommandBuffer>{};
	for (auto q : vkFrameQueuedCommandBuffers[vkCommandGroupUseIndex][(s64)t])
	{
		cbs.AppendAll(q);
	}
	auto si = VkSubmitInfo
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = (u32)waitSems.count,
		.pWaitSemaphores = waitSems.elements,
		.pWaitDstStageMask = waitStages.elements,
		.commandBufferCount = (u32)cbs.count,
		.pCommandBuffers = cbs.elements,
		.signalSemaphoreCount = (u32)signalSems.count,
		.pSignalSemaphores = signalSems.elements,
	};
	VkCheck(vkQueueSubmit(vkQueues[(s64)t], 1, &si, f));
}

// @TODO: This submission code will only work if we do one submission per frame. Fix this!
void SubmitGraphicsCommands()
{
	auto waitSems = MakeStaticArray<VkSemaphore>(
		queues[(s64)QueueType::Transfer].frameSemaphores[swapchain.frameIndex],
		swapchain.imageAcquiredSemaphores[swapchain.frameIndex]);
	auto waitStages = MakeStaticArray<VkPipelineStageFlags>(
		VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	auto signalSems = MakeStaticArray<VkSemaphore>(queues[(s64)QueueType::Graphics].frameSemaphores[swapchain.frameIndex]);
	SubmitCommands(QueueType::Graphics, waitSems, waitStages, signalSems, swapchain.frameFences[swapchain.frameIndex]);
}

void SubmitTransferCommands()
{
	auto signalSems = MakeStaticArray<VkSemaphore>(queues[(s64)QueueType::Transfer].frameSemaphores[swapchain.frameIndex]);
	SubmitCommands(QueueType::Transfer, {}, {}, signalSems, NULL);
}

}

#endif
