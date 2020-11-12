#pragma once

#ifdef VulkanBuild

// @TODO: Should max frames in flight go to a more central header?
namespace GPU::Vulkan
{

struct PhysicalDevice;
struct Device;
struct Swapchain;
struct CommandBuffer;

enum class QueueType
{
	Graphics,
	Transfer,
	Compute,
	Present,
	Count
};

struct Queues
{
	// @TODO: Should be able to store multiple semaphores per frame in case we want to do multiple submits.
	array::Static<VkQueue, s64(QueueType::Count)> queues;
	array::Static<array::Static<array::Array<array::Array<VkCommandBuffer>>, s64(QueueType::Count)>, 2 + 1> submissions; // @TODO
	array::Static<array::Static<VkSemaphore, s64(QueueType::Count)>, 2> submissionSemaphores; // @TODO

	void SubmitGraphicsCommands(array::View<VkCommandBuffer> threadCBs, Swapchain sc, s64 frameIndex);
	void SubmitTransferCommands(array::View<VkCommandBuffer> threadCBs, s64 frameIndex);
	void SubmitCommands(array::View<VkCommandBuffer> threadCBs, QueueType t, array::View<VkSemaphore> waitSems, array::View<VkPipelineStageFlags> waitStages, array::View<VkSemaphore> signalSems, VkFence f);
	void ClearSubmissions(s64 frameIndex);
};

Queues NewQueues(PhysicalDevice pd, Device d);

}

#endif
