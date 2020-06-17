// @TODO: Block pool.
// @TODO: Free unused blocks back to the block pool.
// @TODO: Different block sizes? Small, medium, large, etc.

#include "GPU.h"
#include "Job.h"
#include "Math.h"
#include "Render.h"

#include "Code/Basic/Atomic.h"
#include "Code/Basic/Log.h"

#define GPU_MEMORY_BLOCK_SIZE MegabytesToBytes(256)
#define GPU_MEMORY_RING_SIZE MegabytesToBytes(128)

// @TODO: False sharing?
struct GPUGlobals
{
	struct MemoryAllocators
	{
		GPUMemoryBlockAllocator block[GFX_MEMORY_TYPE_COUNT];
		GPUMemoryRingAllocator cpuToGPURing;
	} memoryAllocators;

	GfxCommandQueue commandQueues[GFX_COMMAND_QUEUE_COUNT];

	GfxCommandPool asyncCommandPools[GFX_COMMAND_QUEUE_COUNT];

	volatile s64 asyncCommandBufferArrayIndices[GFX_COMMAND_QUEUE_COUNT];

	struct ThreadLocalGPUContext
	{
		GfxCommandPool frameCommandPools[GFX_MAX_FRAMES_IN_FLIGHT][GFX_COMMAND_QUEUE_COUNT];

		Array<GPUBackendCommandBuffer> queuedAsyncCommandBuffers[2][GFX_COMMAND_QUEUE_COUNT];
		Array<GPUBackendCommandBuffer> queuedFrameCommandBuffers[GFX_COMMAND_QUEUE_COUNT];
	};
	Array<ThreadLocalGPUContext> threadLocal;
} gpuGlobals;

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

GPUMemoryRingAllocator CreateGPUMemoryRingAllocator(GfxMemoryType memoryType)
{
	auto allocator = GPUMemoryRingAllocator
	{
		.capacity = GPU_MEMORY_RING_SIZE,
		.memoryType = memoryType,
	};
	if (!GfxAllocateMemory(allocator.capacity, memoryType, &allocator.memory))
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

void LogGPUInitialization()
{
	LogPrint(INFO_LOG, "GPU info:\n");
	LogPrint(INFO_LOG, "	Heap indices:\n");
	LogPrint(INFO_LOG, "		GPU only: %d\n", GetGPUMemoryHeapIndex(GFX_GPU_ONLY_MEMORY));
	LogPrint(INFO_LOG, "		CPU-to-GPU: %d\n", GetGPUMemoryHeapIndex(GFX_CPU_TO_GPU_MEMORY));
	LogPrint(INFO_LOG, "		GPU-to-CPU: %d\n", GetGPUMemoryHeapIndex(GFX_GPU_TO_CPU_MEMORY));
	LogPrint(INFO_LOG, "	Memory budget:\n");
	LogPrint(INFO_LOG, "	Buffer-image granularity: %d\n", GetGPUBufferImageGranularity());
	PrintGPUMemoryInfo();
}

void InitializeGPU()
{
	s64 minimumRequiredMemoryPerType[GFX_MEMORY_TYPE_COUNT] = {};
	for (auto i = 0; i < GFX_MEMORY_TYPE_COUNT; i++)
	{
		switch (i)
		{
		case GFX_GPU_ONLY_MEMORY:
		{
			minimumRequiredMemoryPerType[i] += GPU_MEMORY_BLOCK_SIZE + GPU_MEMORY_RING_SIZE;
		} break;
		case GFX_CPU_TO_GPU_MEMORY:
		{
			minimumRequiredMemoryPerType[i] += GPU_MEMORY_BLOCK_SIZE + GPU_MEMORY_RING_SIZE;
		} break;
		case GFX_GPU_TO_CPU_MEMORY:
		{
			minimumRequiredMemoryPerType[i] += MegabytesToBytes(64); // @TODO: Revisit this number once we start using this memory type...
		} break;
		default:
		{
			Abort("Unknown memory type %d.", i);
		} break;
		}
	}
	Assert(CArrayCount(minimumRequiredMemoryPerType) == GFX_MEMORY_TYPE_COUNT); // @TODO: Switch to a fixed size array.
	s64 minimumMemoryRequirementsPerHeap[GPU_MAX_MEMORY_HEAP_COUNT] = {};
	for (auto i = 0; i < GFX_MEMORY_TYPE_COUNT; i++)
	{
		minimumMemoryRequirementsPerHeap[GetGPUMemoryHeapIndex((GfxMemoryType)i)] += minimumRequiredMemoryPerType[i];
	}
	auto memoryInfo = GetGPUMemoryInfo();
	for (auto i = 0; i < memoryInfo.count; i++)
	{
		if (memoryInfo[i].budget < minimumMemoryRequirementsPerHeap[i])
		{
			Abort("Not enough GPU memory for heap %d: require at least %fmb, got %fmb.", i, BytesToMegabytes(minimumMemoryRequirementsPerHeap[i]), BytesToMegabytes(memoryInfo[i].budget));
		}
	}

	for (auto i = 0; i < GFX_MEMORY_TYPE_COUNT; i++)
	{
		gpuGlobals.memoryAllocators.block[i] = CreateGPUMemoryBlockAllocator((GfxMemoryType)i);
	}
	gpuGlobals.memoryAllocators.cpuToGPURing = CreateGPUMemoryRingAllocator(GFX_CPU_TO_GPU_MEMORY);

	for (auto i = 0; i < GFX_COMMAND_QUEUE_COUNT; i++)
	{
		gpuGlobals.asyncCommandPools[i] = GfxCreateCommandPool((GfxCommandQueueType)i);
	}

	ResizeArray(&gpuGlobals.threadLocal, GetWorkerThreadCount());
	for (auto &tl : gpuGlobals.threadLocal)
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
		gpuGlobals.commandQueues[i] = GfxGetCommandQueue((GfxCommandQueueType)i);
	}

	LogGPUInitialization();
}

// @TODO: Buffer-image granularity.

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
			Abort("Failed to allocate GPU memory for block allocator..."); // @TODO
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
	if (lifetime == GPU_RESOURCE_LIFETIME_FRAME && memoryType == GFX_CPU_TO_GPU_MEMORY)
	{
		allocation = AllocateFromGPUMemoryRing(&gpuGlobals.memoryAllocators.ring, memoryRequirements, GetFrameIndex());
	}
	else
	{
		allocation = AllocateFromGPUMemoryBlocks(&gpuGlobals.memoryAllocators.block[memoryType], memoryRequirements);
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

GPUCommandBuffer CreateGPUCommandBuffer(GfxCommandQueueType queueType, GPUResourceLifetime lifetime)
{
	auto commandBuffer = GPUCommandBuffer
	{
		.queueType = queueType,
		.lifetime = lifetime,
	};
	if (lifetime == GPU_RESOURCE_LIFETIME_FRAME)
	{
		commandBuffer.backend = CreateGPUBackendCommandBuffer(gpuGlobals.threadLocal[GetThreadIndex()].frameCommandPools[GetFrameIndex()][queueType]);
	}
	else
	{
		commandBuffer.backend = CreateGPUBackendCommandBuffer(gpuGlobals.asyncCommandPools[queueType]);
	}
	return commandBuffer;
}

void QueueGPUCommandBuffer(GPUCommandBuffer commandBuffer, bool *signalOnCompletion)
{
	GfxEndCommandBuffer(commandBuffer);
	if (commandBuffer.lifetime == GPU_RESOURCE_LIFETIME_FRAME)
	{
		ArrayAppend(&gpuGlobals.threadLocal[GetThreadIndex()].queuedFrameCommandBuffers[commandBuffer.queueType], commandBuffer.backend);
	}
	else
	{
		ArrayAppend(&gpuGlobals.threadLocal[GetThreadIndex()].queuedAsyncCommandBuffers[gpuGlobals.asyncCommandBufferArrayIndices[commandBuffer.queueType]][commandBuffer.queueType], commandBuffer.backend);
	}
}

void SubmitQueuedAsyncGPUCommmandBuffers(GfxCommandQueueType queueType)
{
	auto arrayIndex = gpuGlobals.asyncCommandBufferArrayIndices[queueType];

	if (gpuGlobals.asyncCommandBufferArrayIndices[queueType] == 0)
	{
		gpuGlobals.asyncCommandBufferArrayIndices[queueType] = 1;
	}
	else
	{
		gpuGlobals.asyncCommandBufferArrayIndices[queueType] = 0;
	}

	auto commandBufferCount = 0;
	for (auto &tl : gpuGlobals.threadLocal)
	{
		commandBufferCount += tl.queuedAsyncCommandBuffers[arrayIndex][queueType].count;
	}
	if (commandBufferCount == 0)
	{
		return;
	}
	auto commandBuffers = CreateArray<GPUBackendCommandBuffer>(commandBufferCount);
	auto writeIndex = 0;
	for (auto &tl : gpuGlobals.threadLocal)
	{
		CopyMemory(tl.queuedAsyncCommandBuffers[arrayIndex][queueType].elements, &commandBuffers.elements[writeIndex], tl.queuedAsyncCommandBuffers[arrayIndex][queueType].count * sizeof(GPUBackendCommandBuffer));
		writeIndex += tl.queuedAsyncCommandBuffers[arrayIndex][queueType].count;
		ResizeArray(&tl.queuedAsyncCommandBuffers[arrayIndex][queueType], 0);
		// @TODO: Change capacity to some default value?
	}

	auto submitInfo = GfxSubmitInfo
	{
		.commandBuffers = commandBuffers,
	};
	GfxSubmitCommandBuffers(gpuGlobals.commandQueues[queueType], submitInfo, NULL);
}

GfxSemaphore SubmitQueuedFrameGPUCommandBuffers(GfxCommandQueueType queueType, Array<GfxSemaphore> waitSemaphores, Array<GfxPipelineStageFlags> waitStages, GfxFence fence)
{
	auto commandBufferCount = 0;
	for (auto &tl : gpuGlobals.threadLocal)
	{
		commandBufferCount += tl.queuedFrameCommandBuffers[queueType].count;
	}
	Assert(commandBufferCount != 0);
	auto commandBuffers = CreateArray<GPUBackendCommandBuffer>(commandBufferCount);
	auto writeIndex = 0;
	for (auto &tl : gpuGlobals.threadLocal)
	{
		CopyMemory(tl.queuedFrameCommandBuffers[queueType].elements, &commandBuffers.elements[writeIndex], tl.queuedFrameCommandBuffers[queueType].count * sizeof(GPUBackendCommandBuffer));
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
	GfxSubmitCommandBuffers(gpuGlobals.commandQueues[queueType], submitInfo, fence);
	return result;
}

GfxSemaphore SubmitQueuedGPUCommandBuffers(GfxCommandQueueType queueType, Array<GfxSemaphore> frameWaitSemaphores, Array<GfxPipelineStageFlags> frameWaitStages, GfxFence frameFence)
{
	SubmitQueuedAsyncGPUCommmandBuffers(queueType);
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
	Clear(&gpuGlobals.memoryAllocators.cpuToGPURing);
}

void ClearGPUCommandPoolsForFrameIndex(s64 frameIndex)
{
	for (auto &tl : gpuGlobals.threadLocal)
	{
		GfxResetCommandPool(tl.frameCommandPools[frameIndex][GFX_GRAPHICS_COMMAND_QUEUE]);
		GfxResetCommandPool(tl.frameCommandPools[frameIndex][GFX_TRANSFER_COMMAND_QUEUE]);
		GfxResetCommandPool(tl.frameCommandPools[frameIndex][GFX_COMPUTE_COMMAND_QUEUE]);
	}
}

#if 0
GfxSemaphore SubmitQueuedGPUTransferCommands()
{
	SwitchAtomicDoubleBuffer(&gpuGlobals.transferDoubleBuffer);

	auto semaphore = GfxCreateSemaphore();
	GfxSubmitInfo submitInfo =
	{
		.commandBuffers = CreateArray(gpuGlobals.transferDoubleBuffer.readBufferElementCount, gpuGlobals.transferDoubleBuffer.readBuffer->data),
		.signalSemaphores = CreateArray(1, &semaphore),
	};
	GfxSubmitCommandBuffers(GFX_TRANSFER_COMMAND_QUEUE, submitInfo, NULL);
	return semaphore;
}
#endif
