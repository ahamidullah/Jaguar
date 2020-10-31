#pragma once

#ifdef VulkanBuild

// @TODO: Should max frames in flight go to a more central header?
#include "Swapchain.h"

namespace GPU
{

struct Queue
{
	// @TODO: Should be able to store multiple semaphores per frame in case we want to do multiple submits.
	StaticArray<VkSemaphore, _MaxFramesInFlight> frameSemaphores;
};

enum class QueueType
{
	Graphics,
	Transfer,
	Compute,
	Present,
	Count
};

extern StaticArray<Queue, (s64)QueueType::Count> queues;

void InitializeQueues();
void SubmitTransferCommands();
void SubmitGraphicsCommands();

}

#endif
