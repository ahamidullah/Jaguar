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
	StaticArray<VkQueue, s64(QueueType::Count)> queues;
	StaticArray<StaticArray<Array<Array<VkCommandBuffer>>, s64(QueueType::Count)>, 2 + 1> submissions; // @TODO
	StaticArray<StaticArray<VkSemaphore, s64(QueueType::Count)>, 2> submissionSemaphores; // @TODO

	void SubmitGraphicsCommands(ArrayView<VkCommandBuffer> threadCBs, Swapchain sc, s64 frameIndex);
	void SubmitTransferCommands(ArrayView<VkCommandBuffer> threadCBs, s64 frameIndex);
	void SubmitCommands(ArrayView<VkCommandBuffer> threadCBs, QueueType t, ArrayView<VkSemaphore> waitSems, ArrayView<VkPipelineStageFlags> waitStages, ArrayView<VkSemaphore> signalSems, VkFence f);
	void ClearSubmissions(s64 frameIndex);
};

Queues NewQueues(PhysicalDevice pd, Device d);

}

#endif
