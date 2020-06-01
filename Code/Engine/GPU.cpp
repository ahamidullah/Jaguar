// @TODO: Block pool.
// @TODO: Free unused blocks back to the block pool.
// @TODO: Different block sizes? Small, medium, large, etc.

#include "GPU.h"
#include "Job.h"
#include "Math.h"
#include "Render.h"

#include "Code/Basic/Atomic.h"
#include "Code/Basic/Log.h"

struct GPUContext
{
	// @TODO: False sharing?
	struct MemoryAllocators
	{
		// Due to buffer-image granularity requirements, allocations for buffers and images must be kept seperate.

		GPUMemoryBlockAllocator bufferBlock;
		GPUMemoryRingAllocator bufferRing;

		GPUMemoryBlockAllocator imageBlock;
		GPUMemoryRingAllocator imageRing;
	} memoryAllocators[GFX_MEMORY_TYPE_COUNT];

	// @TODO: False sharing?
	GfxCommandQueue commandQueues[GFX_COMMAND_QUEUE_COUNT];

	// @TODO: False sharing?
	GfxCommandPool asyncCommandPools[GFX_COMMAND_QUEUE_COUNT];

	volatile s64 asyncCommandBufferArrayIndex = 0;

	struct ThreadLocalGPUContext
	{
		// @TODO: False sharing?
		GfxCommandPool frameCommandPools[GFX_MAX_FRAMES_IN_FLIGHT][GFX_COMMAND_QUEUE_COUNT];

		Array<GfxCommandBuffer> queuedAsyncCommandBuffers[2][GFX_COMMAND_QUEUE_COUNT];
		Array<GfxCommandBuffer> queuedFrameCommandBuffers[GFX_COMMAND_QUEUE_COUNT];
	};
	Array<ThreadLocalGPUContext> threadLocal;
} gpuContext;

enum GPUResourceType
{
	GPU_BUFFER_RESOURCE,
	GPU_IMAGE_RESOURCE,
};

bool GPUMemoryTypeIsCPUVisible(GfxMemoryType memoryType)
{
	if (memoryType == GFX_CPU_TO_GPU_MEMORY || memoryType == GFX_GPU_TO_CPU_MEMORY)
	{
		return true;
	}
	return false;
}

GPUMemoryBlockAllocator CreateGPUMemoryBlockAllocator(GfxMemoryType memoryType)
{
	auto allocator = GPUMemoryBlockAllocator
	{
		.baseBlock = NULL,
		.activeBlock = NULL,
		.memoryType = memoryType,
	};
	allocator.mutex = CreateMutex();
	return allocator;
}

GPUMemoryRingAllocator CreateGPUMemoryRingAllocator(s64 capacity, GfxMemoryType memoryType)
{
	auto allocator = GPUMemoryRingAllocator
	{
		.capacity = capacity,
		.memoryType = memoryType,
	};
	if (!GfxAllocateMemory(capacity, memoryType, &allocator.memory))
	{
		Abort("Failed to allocate ring allocator memory.");
	}
	if (GPUMemoryTypeIsCPUVisible(memoryType))
	{
		allocator.mappedMemory = GfxMapMemory(allocator.memory, allocator.capacity, 0);
	}
	else
	{
		allocator.mappedMemory = NULL;
	}
	return allocator;
}

void InitializeGPU()
{
	gpuContext.memoryAllocators[GFX_GPU_ONLY_MEMORY].bufferBlock = CreateGPUMemoryBlockAllocator(GFX_GPU_ONLY_MEMORY);
	gpuContext.memoryAllocators[GFX_GPU_ONLY_MEMORY].bufferRing = CreateGPUMemoryRingAllocator(Megabyte(128), GFX_GPU_ONLY_MEMORY);

	gpuContext.memoryAllocators[GFX_GPU_ONLY_MEMORY].imageBlock = CreateGPUMemoryBlockAllocator(GFX_GPU_ONLY_MEMORY);
	gpuContext.memoryAllocators[GFX_GPU_ONLY_MEMORY].imageRing = CreateGPUMemoryRingAllocator(Megabyte(256), GFX_GPU_ONLY_MEMORY);

	gpuContext.memoryAllocators[GFX_CPU_TO_GPU_MEMORY].bufferBlock = CreateGPUMemoryBlockAllocator(GFX_CPU_TO_GPU_MEMORY);
	gpuContext.memoryAllocators[GFX_CPU_TO_GPU_MEMORY].bufferRing = CreateGPUMemoryRingAllocator(Megabyte(128), GFX_CPU_TO_GPU_MEMORY);

	gpuContext.memoryAllocators[GFX_CPU_TO_GPU_MEMORY].imageBlock = CreateGPUMemoryBlockAllocator(GFX_CPU_TO_GPU_MEMORY);
	gpuContext.memoryAllocators[GFX_CPU_TO_GPU_MEMORY].imageRing = CreateGPUMemoryRingAllocator(Megabyte(256), GFX_CPU_TO_GPU_MEMORY);

	gpuContext.memoryAllocators[GFX_GPU_TO_CPU_MEMORY].bufferBlock = CreateGPUMemoryBlockAllocator(GFX_GPU_TO_CPU_MEMORY);
	gpuContext.memoryAllocators[GFX_GPU_TO_CPU_MEMORY].bufferRing = CreateGPUMemoryRingAllocator(Megabyte(128), GFX_GPU_TO_CPU_MEMORY);

	gpuContext.memoryAllocators[GFX_GPU_TO_CPU_MEMORY].imageBlock = CreateGPUMemoryBlockAllocator(GFX_GPU_TO_CPU_MEMORY);
	gpuContext.memoryAllocators[GFX_GPU_TO_CPU_MEMORY].imageRing = CreateGPUMemoryRingAllocator(Megabyte(256), GFX_GPU_TO_CPU_MEMORY);

	for (auto i = 0; i < GFX_COMMAND_QUEUE_COUNT; i++)
	{
		gpuContext.asyncCommandPools[i] = GfxCreateCommandPool((GfxCommandQueueType)i);
	}

	ResizeArray(&gpuContext.threadLocal, GetWorkerThreadCount());
	for (auto &tl : gpuContext.threadLocal)
	{
		for (auto &cp : tl.frameCommandPools)
		{
			for (auto i = 0; i < GFX_COMMAND_QUEUE_COUNT; i++)
			{
				cp[i] = GfxCreateCommandPool((GfxCommandQueueType)i);
			}
		}
	}

	for (auto i = 0; i < GFX_COMMAND_QUEUE_COUNT; i++)
	{
		gpuContext.commandQueues[i] = GfxGetCommandQueue((GfxCommandQueueType)i);
	}
}

// @TODO: Get rid of binding -- doesn't fit with DX12.
// @TODO: Mapping?????
// @TODO: Buffer image granularity???????

GPUMemoryAllocation *AllocateFromGPUMemoryBlocks(GPUMemoryBlockAllocator *allocator, GfxMemoryRequirements memoryRequirements)
{
	LockMutex(&allocator->mutex);
	Defer(UnlockMutex(&allocator->mutex));

	Assert(memoryRequirements.size < GPU_MEMORY_BLOCK_SIZE);

	// @TODO: Try to allocate out of the freed allocations.
	if (!allocator->activeBlock || allocator->activeBlock->frontier + memoryRequirements.size > GPU_MEMORY_BLOCK_SIZE)
	{
		// Need to allocate a new block.
		auto newBlock = (GPUMemoryBlock *)malloc(sizeof(GPUMemoryBlock)); // @TODO
		if (!GfxAllocateMemory(GPU_MEMORY_BLOCK_SIZE, allocator->memoryType, &newBlock->memory))
		{
			Abort("Failed to allocate GPU memory for block allocator...");
			return NULL;
		}
		newBlock->frontier = 0;
		newBlock->allocationCount = 0;
		if (GPUMemoryTypeIsCPUVisible(allocator->memoryType))
		{
			newBlock->mappedMemory = GfxMapMemory(newBlock->memory, GPU_MEMORY_BLOCK_SIZE, 0);
		}
		else
		{
			newBlock->mappedMemory = NULL;
		}
		newBlock->next = NULL;
		if (allocator->baseBlock)
		{
			allocator->activeBlock->next = newBlock;
			allocator->activeBlock = newBlock;
		}
		else
		{
			allocator->baseBlock = newBlock;
			allocator->activeBlock = allocator->baseBlock;
		}
	}

	// Allocate out of the active block's frontier.
	auto newAllocation = &allocator->activeBlock->allocations[allocator->activeBlock->allocationCount++];
	auto allocationStartOffset = AlignTo(allocator->activeBlock->frontier, memoryRequirements.alignment);
	*newAllocation =
	{
		.memory = allocator->activeBlock->memory,
		.offset = allocationStartOffset,
	};
	if (GPUMemoryTypeIsCPUVisible(allocator->memoryType))
	{
		newAllocation->mappedMemory = (char *)allocator->activeBlock->mappedMemory + allocationStartOffset;
	} else
	{
		newAllocation->mappedMemory = NULL;
	}
	Assert(allocator->activeBlock->allocationCount < GPU_MAX_MEMORY_ALLOCATIONS_PER_BLOCK);
	allocator->activeBlock->frontier = allocationStartOffset + memoryRequirements.size;
	Assert(allocator->activeBlock->frontier <= GPU_MEMORY_BLOCK_SIZE);
	return newAllocation;
}

GPUMemoryAllocation *AllocateFromGPUMemoryRing(GPUMemoryRingAllocator *allocator, GfxMemoryRequirements memoryRequirements, s64 frameIndex)
{
	auto newEnd = 0, oldEnd = 0, allocationStart = 0, allocationSize = 0;
	do
	{
		oldEnd = allocator->end;
		auto alignmentOffset = AlignmentOffset(oldEnd, memoryRequirements.alignment);
		allocationSize = alignmentOffset + memoryRequirements.size;
		if (allocator->size + allocationSize > allocator->capacity)
		{
			// @TODO: Fallback to block allocators.
			Abort("Ring buffer ran out of space.\n");
		}
		allocationStart = (oldEnd + alignmentOffset) % allocator->capacity;
		newEnd = (allocationStart + memoryRequirements.size) % allocator->capacity;
	} while (AtomicCompareAndSwap(&allocator->end, oldEnd, newEnd) != oldEnd);

	AtomicFetchAndAdd(&allocator->size, allocationSize);
	AtomicFetchAndAdd(&allocator->frameSizes[frameIndex], allocationSize);

	auto allocationIndex = AtomicFetchAndAdd(&allocator->allocationCounts[frameIndex], 1);
	auto allocation = &allocator->allocations[frameIndex][allocationIndex];
	allocation->memory = allocator->memory;
	allocation->offset = allocationStart;
	if (GPUMemoryTypeIsCPUVisible(allocator->memoryType))
	{
		allocation->mappedMemory = (char *)allocator->mappedMemory + allocationStart;
	}
	return allocation;
}

GPUMemoryAllocation *AllocateGPUMemory(GPUResourceType resourceType, GfxMemoryRequirements memoryRequirements, GfxMemoryType memoryType, GPUResourceLifetime lifetime, void **mappedMemory)
{
	auto allocation = (GPUMemoryAllocation *){};
	if (lifetime == GPU_RESOURCE_LIFETIME_FRAME)
	{
		if (resourceType == GPU_BUFFER_RESOURCE)
		{
			allocation = AllocateFromGPUMemoryRing(&gpuContext.memoryAllocators[memoryType].bufferRing, memoryRequirements, GetFrameIndex());
		}
		else
		{
			allocation = AllocateFromGPUMemoryRing(&gpuContext.memoryAllocators[memoryType].imageRing, memoryRequirements, GetFrameIndex());
		}
	}
	else
	{
		if (resourceType == GPU_BUFFER_RESOURCE)
		{
			allocation = AllocateFromGPUMemoryBlocks(&gpuContext.memoryAllocators[memoryType].bufferBlock, memoryRequirements);
		}
		else
		{
			allocation = AllocateFromGPUMemoryBlocks(&gpuContext.memoryAllocators[memoryType].imageBlock, memoryRequirements);
		}
	}
	Assert(allocation);
	if (mappedMemory)
	{
		Assert(GPUMemoryTypeIsCPUVisible(memoryType));
		*mappedMemory = allocation->mappedMemory;
	}
	return allocation;
}

GfxBuffer CreateGPUBuffer(s64 size, GfxBufferUsageFlags usage, GfxMemoryType memoryType, GPUResourceLifetime lifetime, void **mappedMemory)
{
	auto buffer = GfxCreateBuffer(size, usage);
	auto memoryRequirements = GfxGetBufferMemoryRequirements(buffer);
	auto allocation = AllocateGPUMemory(GPU_BUFFER_RESOURCE, memoryRequirements, memoryType, lifetime, mappedMemory);
	GfxBindBufferMemory(buffer, allocation->memory, allocation->offset);
	return buffer;
}

GfxImage CreateGPUImage(s64 width, s64 height, GfxFormat format, GfxImageLayout initialLayout, GfxImageUsageFlags usage, GfxSampleCount sampleCount, GfxMemoryType memoryType, GPUResourceLifetime lifetime, void **mappedMemory)
{
	auto image = GfxCreateImage(width, height, format, initialLayout, usage, sampleCount);
	auto memoryRequirements = GfxGetImageMemoryRequirements(image);
	auto allocation = AllocateGPUMemory(GPU_IMAGE_RESOURCE, memoryRequirements, memoryType, lifetime, mappedMemory);
	Assert(allocation);
	GfxBindImageMemory(image, allocation->memory, allocation->offset);
	return image;
}

GfxCommandBuffer CreateGPUCommandBuffer(GfxCommandQueueType queueType, GPUResourceLifetime lifetime)
{
	if (lifetime == GPU_RESOURCE_LIFETIME_FRAME)
	{
		return GfxCreateCommandBuffer(gpuContext.threadLocal[GetThreadIndex()].frameCommandPools[GetFrameIndex()][queueType]);
	}
	return GfxCreateCommandBuffer(gpuContext.asyncCommandPools[queueType]);
}

void QueueGPUCommandBuffer(GfxCommandBuffer commandBuffer, GfxCommandQueueType queueType, GPUResourceLifetime lifetime, bool *signalOnCompletion) // @TODO: Store the queue and lifetime.
{
	GfxEndCommandBuffer(commandBuffer);
	if (lifetime == GPU_RESOURCE_LIFETIME_FRAME)
	{
		ArrayAppend(&gpuContext.threadLocal[GetThreadIndex()].queuedFrameCommandBuffers[queueType], commandBuffer);
		return;
	}
	ArrayAppend(&gpuContext.threadLocal[GetThreadIndex()].queuedAsyncCommandBuffers[gpuContext.asyncCommandBufferArrayIndex][queueType], commandBuffer);
}

void SubmitQueuedAsyncGPUCommmandBuffers(GfxCommandQueueType queueType)
{
	auto arrayIndex = gpuContext.asyncCommandBufferArrayIndex;

	// WRONG. NEED ONE PER QUEUE.
	if (gpuContext.asyncCommandBufferArrayIndex == 0)
	{
		gpuContext.asyncCommandBufferArrayIndex = 1;
	}
	else
	{
		gpuContext.asyncCommandBufferArrayIndex = 0;
	}

	auto commandBufferCount = 0;
	for (auto &tl : gpuContext.threadLocal)
	{
		commandBufferCount += tl.queuedAsyncCommandBuffers[arrayIndex][queueType].count;
	}
	if (commandBufferCount == 0)
	{
		return;
	}
	auto commandBuffers = CreateArray<GfxCommandBuffer>(commandBufferCount);
	auto writeIndex = 0;
	for (auto &tl : gpuContext.threadLocal)
	{
		CopyMemory(tl.queuedAsyncCommandBuffers[arrayIndex][queueType].elements, &commandBuffers.elements[writeIndex], tl.queuedAsyncCommandBuffers[arrayIndex][queueType].count * sizeof(GfxCommandBuffer));
		writeIndex += tl.queuedAsyncCommandBuffers[arrayIndex][queueType].count;
		ResizeArray(&tl.queuedAsyncCommandBuffers[arrayIndex][queueType], 0);
		// @TODO: Change capacity to some default value?
	}

	auto submitInfo = GfxSubmitInfo
	{
		.commandBuffers = commandBuffers,
	};
	GfxSubmitCommandBuffers(gpuContext.commandQueues[queueType], submitInfo, NULL);
}

GfxSemaphore SubmitQueuedFrameGPUCommandBuffers(GfxCommandQueueType queueType, Array<GfxSemaphore> waitSemaphores, Array<GfxPipelineStageFlags> waitStages, GfxFence fence)
{
	auto commandBufferCount = 0;
	for (auto &tl : gpuContext.threadLocal)
	{
		commandBufferCount += tl.queuedFrameCommandBuffers[queueType].count;
	}
	Assert(commandBufferCount != 0);
	auto commandBuffers = CreateArray<GfxCommandBuffer>(commandBufferCount);
	auto writeIndex = 0;
	for (auto &tl : gpuContext.threadLocal)
	{
		CopyMemory(tl.queuedFrameCommandBuffers[queueType].elements, &commandBuffers.elements[writeIndex], tl.queuedFrameCommandBuffers[queueType].count * sizeof(GfxCommandBuffer));
		writeIndex += tl.queuedFrameCommandBuffers[queueType].count;
		ResizeArray(&tl.queuedFrameCommandBuffers[queueType], 0);
		// @TODO: Change capacity to some default value?
	}

	auto result = GfxCreateSemaphore();
	auto submitInfo = GfxSubmitInfo
	{
		.commandBuffers = commandBuffers,
		.waitStages = waitStages,
		.waitSemaphores = waitSemaphores,
		.signalSemaphores = CreateArray(1, &result),
	};
	GfxSubmitCommandBuffers(gpuContext.commandQueues[queueType], submitInfo, fence);
	return result;
}

GfxSemaphore SubmitQueuedGPUCommandBuffers(GfxCommandQueueType queueType, Array<GfxSemaphore> frameWaitSemaphores, Array<GfxPipelineStageFlags> frameWaitStages, GfxFence frameFence)
{
	if (queueType == GFX_TRANSFER_COMMAND_QUEUE)
	{
		SubmitQueuedAsyncGPUCommmandBuffers(queueType);
	}

	return SubmitQueuedFrameGPUCommandBuffers(queueType, frameWaitSemaphores, frameWaitStages, frameFence);
}

void ClearGPUMemoryForFrameIndex(s64 frameIndex)
{
	auto Clear = [frameIndex](GPUMemoryRingAllocator *allocator)
	{
		allocator->start = (allocator->start + allocator->frameSizes[frameIndex]) % allocator->capacity;
		allocator->frameSizes[frameIndex] = 0;
		allocator->allocationCounts[frameIndex] = 0;
		allocator->size = 0;
	};
	for (auto i = 0; i < GFX_MEMORY_TYPE_COUNT; i++)
	{
		Clear(&gpuContext.memoryAllocators[i].bufferRing);
		Clear(&gpuContext.memoryAllocators[i].imageRing);
		// @TODO: Clear overflow.
	}
}

void ClearGPUCommandPoolsForFrameIndex(s64 frameIndex)
{
	for (auto &tl : gpuContext.threadLocal)
	{
		GfxResetCommandPool(tl.frameCommandPools[frameIndex][GFX_GRAPHICS_COMMAND_QUEUE]);
		GfxResetCommandPool(tl.frameCommandPools[frameIndex][GFX_TRANSFER_COMMAND_QUEUE]);
		GfxResetCommandPool(tl.frameCommandPools[frameIndex][GFX_COMPUTE_COMMAND_QUEUE]);
	}
}

#if 0
GfxSemaphore SubmitQueuedGPUTransferCommands()
{
	SwitchAtomicDoubleBuffer(&gpuContext.transferDoubleBuffer);

	auto semaphore = GfxCreateSemaphore();
	GfxSubmitInfo submitInfo =
	{
		.commandBuffers = CreateArray(gpuContext.transferDoubleBuffer.readBufferElementCount, gpuContext.transferDoubleBuffer.readBuffer->data),
		.signalSemaphores = CreateArray(1, &semaphore),
	};
	GfxSubmitCommandBuffers(GFX_TRANSFER_COMMAND_QUEUE, submitInfo, NULL);
	return semaphore;
}
#endif
