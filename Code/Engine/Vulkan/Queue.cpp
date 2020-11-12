#ifdef VulkanBuild

#include "Queue.h"

namespace GPU::Vulkan
{

Queues NewQueues(PhysicalDevice pd, Device d)
{
	auto q = Queues{};
	for (auto i = 0; i < s64(QueueType::Count); i += 1)
	{
		vkGetDeviceQueue(d.device, pd.queueFamilies[i], 0, &q.queues[i]); // No return.
	}
	auto ci = VkSemaphoreCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};
	for (auto i = 0; i < MaxFramesInFlight; i += 1)
	{
		for (auto j = 0; j < s64(QueueType::Count); j += 1)
		{
			Check(vkCreateSemaphore(d.device, &ci, NULL, &q.submissionSemaphores[i][j]));
		}
	}
	for (auto i = 0; i < MaxFramesInFlight + 1; i += 1) // @TODO
	{
		for (auto j = 0; j < s64(QueueType::Count); j += 1)
		{
			q.submissions[i][j] = array::NewIn<array::Array<VkCommandBuffer>>(Memory::GlobalHeap(), WorkerThreadCount());
			for (auto k = 0; k < WorkerThreadCount(); k += 1)
			{
				q.submissions[i][j][k].SetAllocator(Memory::GlobalHeap());
			}
		}
	}
	return q;
}

// @TODO: This submission code will only work if we do one submission per frame. Fix this!
void Queues::SubmitGraphicsCommands(array::View<VkCommandBuffer> threadCBs, Swapchain sc, s64 frameIndex)
{
	auto waitSems = array::MakeStatic<VkSemaphore>(
		this->submissionSemaphores[frameIndex][s64(QueueType::Transfer)],
		sc.imageAcquiredSemaphores[frameIndex]);
	auto waitStages = array::MakeStatic<VkPipelineStageFlags>(
		VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	auto signalSems = array::MakeStatic<VkSemaphore>(this->submissionSemaphores[frameIndex][s64(QueueType::Graphics)]);
	this->SubmitCommands(threadCBs, QueueType::Graphics, waitSems, waitStages, signalSems, sc.frameFences[frameIndex]);
}

void Queues::SubmitTransferCommands(array::View<VkCommandBuffer> threadCBs, s64 frameIndex)
{
	auto signalSems = array::MakeStatic<VkSemaphore>(this->submissionSemaphores[frameIndex][s64(QueueType::Transfer)]);
	this->SubmitCommands(threadCBs, QueueType::Transfer, {}, {}, signalSems, NULL);
}

void Queues::SubmitCommands(array::View<VkCommandBuffer> threadCBs, QueueType t, array::View<VkSemaphore> waitSems, array::View<VkPipelineStageFlags> waitStages, array::View<VkSemaphore> signalSems, VkFence f)
{
	auto cbs = array::NewWithCapacity<VkCommandBuffer>(threadCBs.count);
	for (auto i = 0; i < threadCBs.count; i += 1)
	{
		if (!threadCBs[i])
		{
			continue;
		}
		Check(vkEndCommandBuffer(threadCBs[i]));
		this->submissions[vkCommandGroupUseIndex][s64(t)][i].Append(threadCBs[i]);
		cbs.Append(threadCBs[i]);
	}
	auto si = VkSubmitInfo
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = u32(waitSems.count),
		.pWaitSemaphores = waitSems.elements,
		.pWaitDstStageMask = waitStages.elements,
		.commandBufferCount = u32(cbs.count),
		.pCommandBuffers = cbs.elements,
		.signalSemaphoreCount = u32(signalSems.count),
		.pSignalSemaphores = signalSems.elements,
	};
	Check(vkQueueSubmit(this->queues[s64(t)], 1, &si, f));
}

void Queues::ClearSubmissions(s64 frameIndex)
{
	for (auto tl : this->submissions[vkCommandGroupFreeIndex])
	{
		for (auto &s : tl)
		{
			s.Resize(0);
		}
	}
}

}

#endif
