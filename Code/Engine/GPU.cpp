GPUMemoryBlockAllocator CreateGPUMemoryBlockAllocator(u32 blockSize, GfxMemoryType memoryType)
{
	GPUMemoryBlockAllocator allocator =
	{
		.blockSize = blockSize,
		.baseBlock = NULL,
		.activeBlock = NULL,
		.memoryType = memoryType,
	};
	CreateMutex(&allocator.mutex);
	return allocator;
}

GPUMemoryRingAllocator CreateGPUMemoryRingAllocator(u32 capacity, GfxMemoryType memoryType)
{
	GPUMemoryRingAllocator allocator =
	{
		.memoryType = memoryType,
		.capacity = capacity,
		.bottom = 0,
		.top = 0,
	};
	if (!GfxAllocateMemory(capacity, memoryType, &allocator.memory))
	{
		Abort("Failed to allocate ring allocator memory");
	}
	if (memoryType == GFX_HOST_MEMORY)
	{
		allocator.mappedPointer = GfxMapMemory(allocator.memory, allocator.capacity, 0);
	}
	else
	{
		allocator.mappedPointer = NULL;
	}
	return allocator;
}

struct ThreadLocalGPUContext
{
	struct GPUCommandPools
	{
		GfxCommandPool graphics;
		GfxCommandPool transfer;
		GfxCommandPool compute;
	} commandPools[GFX_MAX_FRAMES_IN_FLIGHT];
};

struct GPUContext
{
	struct MemoryAllocators
	{
		GPUMemoryBlockAllocator deviceBlockBuffer;
		GPUMemoryRingAllocator deviceRingBuffer;
		GPUMemoryBlockAllocator hostBlockBuffer;
		GPUMemoryRingAllocator hostRingBuffer;
		GPUMemoryBlockAllocator deviceBlockImage;
	} memoryAllocators;

	struct CommandPools
	{
		GfxCommandPool graphics;
		GfxCommandPool transfer;
		GfxCommandPool compute;
	} commandPools;

	AtomicDoubleBuffer<GfxCommandBuffer, 100> transferDoubleBuffer; // @TODO: Fix size.

	Array<ThreadLocalGPUContext> threadLocal;
} gpuContext;

void InitializeGPU()
{
	gpuContext.memoryAllocators.deviceBlockBuffer = CreateGPUMemoryBlockAllocator(Megabyte(8), GFX_DEVICE_MEMORY);
	gpuContext.memoryAllocators.deviceRingBuffer = CreateGPUMemoryRingAllocator(Megabyte(8), GFX_DEVICE_MEMORY);
	gpuContext.memoryAllocators.hostBlockBuffer = CreateGPUMemoryBlockAllocator(Megabyte(256), GFX_HOST_MEMORY);
	gpuContext.memoryAllocators.hostRingBuffer = CreateGPUMemoryRingAllocator(Megabyte(256), GFX_HOST_MEMORY);
	gpuContext.memoryAllocators.deviceBlockImage = CreateGPUMemoryBlockAllocator(Megabyte(256), GFX_DEVICE_MEMORY);

	gpuContext.commandPools.graphics = GfxCreateCommandPool(GFX_GRAPHICS_COMMAND_QUEUE);
	gpuContext.commandPools.transfer = GfxCreateCommandPool(GFX_TRANSFER_COMMAND_QUEUE);
	gpuContext.commandPools.compute = GfxCreateCommandPool(GFX_COMPUTE_COMMAND_QUEUE);

	Resize(&gpuContext.threadLocal, GetWorkerThreadCount());
	for (auto i = 0; i < GetWorkerThreadCount(); i++)
	{
		for (auto j = 0; j < GFX_MAX_FRAMES_IN_FLIGHT; j++)
		{
			gpuContext.threadLocal[i].commandPools[j].graphics = GfxCreateCommandPool(GFX_GRAPHICS_COMMAND_QUEUE);
			gpuContext.threadLocal[i].commandPools[j].transfer = GfxCreateCommandPool(GFX_TRANSFER_COMMAND_QUEUE);
			gpuContext.threadLocal[i].commandPools[j].compute = GfxCreateCommandPool(GFX_COMPUTE_COMMAND_QUEUE);
		}
	}
}

GPUMemoryAllocation *AllocateFromGPUMemoryBlocks(GPUMemoryBlockAllocator *allocator, GfxMemoryRequirements memoryRequirements)
{
	LockMutex(&allocator->mutex);
	Assert(memoryRequirements.size < allocator->blockSize);
	// @TODO: Try to allocate out of the freed allocations.
	if (!allocator->activeBlock || allocator->activeBlock->frontier + memoryRequirements.size > allocator->blockSize)
	{
		// Need to allocate a new block.
		auto newBlock = (GPUMemoryBlock *)malloc(sizeof(GPUMemoryBlock)); // @TODO
		if (!GfxAllocateMemory(allocator->blockSize, allocator->memoryType, &newBlock->memory))
		{
			free(newBlock); // @TODO
			return NULL;
		}
		newBlock->frontier = 0;
		newBlock->allocationCount = 0;
		if (allocator->memoryType == GFX_HOST_MEMORY)
		{
			newBlock->mappedPointer = GfxMapMemory(newBlock->memory, allocator->blockSize, 0);
		}
		else
		{
			newBlock->mappedPointer = NULL;
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
	GPUMemoryAllocation *newAllocation = &allocator->activeBlock->allocations[allocator->activeBlock->allocationCount++];
	u32 allocationStartOffset = AlignU32(allocator->activeBlock->frontier, memoryRequirements.alignment);
	*newAllocation = {
		.memory = allocator->activeBlock->memory,
		.offset = allocationStartOffset,
	};
	if (allocator->memoryType == GFX_HOST_MEMORY)
	{
		newAllocation->mappedPointer = (char *)allocator->activeBlock->mappedPointer + allocationStartOffset;
		//ConsolePrint("MAPPED %x %u\n", newAllocation->mappedPointer, allocationStartOffset);
	} else {
		newAllocation->mappedPointer = NULL;
	}
	Assert(allocator->activeBlock->allocationCount < GPU_MAX_MEMORY_ALLOCATIONS_PER_BLOCK);
	allocator->activeBlock->frontier = allocationStartOffset + memoryRequirements.size;
	Assert(allocator->activeBlock->frontier <= allocator->blockSize);
	UnlockMutex(&allocator->mutex);
	return newAllocation;
}

GPUMemoryAllocation *AllocateFromGPUMemoryRing(GPUMemoryRingAllocator *allocator, GfxMemoryRequirements memoryRequirements, u32 frameIndex)
{
	auto newTop = 0, oldTop = 0, allocationStart = 0, allocationSize = 0;
	do
	{
		oldTop = allocator->top;
		auto alignmentOffset = AlignmentOffset(oldTop, memoryRequirements.alignment);
		allocationSize = alignmentOffset + memoryRequirements.size;
		if (allocator->size + allocationSize > allocator->capacity)
		{
			// @TODO: Fallback to block allocators.
			Abort("Ring buffer ran out of space.\n");
		}
		allocationStart = (oldTop + alignmentOffset) % allocator->capacity;
		newTop = (allocationStart + memoryRequirements.size) % allocator->capacity;
		//ConsolePrint("allocator->top: %d, oldTop: %d, newTop: %d\n", allocator->top, oldTop, newTop);
	} while (AtomicCompareAndSwap((s32 *)&allocator->top, oldTop, newTop) != oldTop);
	AtomicFetchAndAdd((s32 *)&allocator->size, allocationSize);
	AtomicFetchAndAdd((s32 *)&allocator->frameSizes[frameIndex], allocationSize);

	auto allocationIndex = AtomicFetchAndAdd((s32 *)&allocator->allocationCounts[frameIndex], 1);
	auto allocation = &allocator->allocations[frameIndex][allocationIndex];
	allocation->memory = allocator->memory;
	allocation->offset = allocationStart;
	if (allocator->memoryType == GFX_HOST_MEMORY)
	{
		allocation->mappedPointer = (char *)allocator->mappedPointer + allocationStart;
	}
	return allocation;
}

void ClearGPUMemoryForFrameIndex(u32 frameIndex)
{
	auto clear = [frameIndex](GPUMemoryRingAllocator *allocator)
	{
		allocator->bottom = (allocator->bottom + allocator->frameSizes[frameIndex]) % allocator->capacity;
		allocator->frameSizes[frameIndex] = 0;
		allocator->allocationCounts[frameIndex] = 0;
		allocator->size = 0;
	};
	clear(&gpuContext.memoryAllocators.hostRingBuffer);
	clear(&gpuContext.memoryAllocators.deviceRingBuffer);
}

GfxBuffer CreateGPUBuffer(u32 size, GfxBufferUsageFlags usage, GfxMemoryType memoryType, GPUResourceLifetime lifetime, void **mappedPointer = NULL)
{
	auto buffer = GfxCreateBuffer(size, usage);
	auto memoryRequirements = GfxGetBufferMemoryRequirements(buffer);
	GPUMemoryAllocation *allocation;
	if (lifetime == GPU_RESOURCE_LIFETIME_FRAME)
	{
		if (memoryType == GFX_HOST_MEMORY)
		{
			allocation = AllocateFromGPUMemoryRing(&gpuContext.memoryAllocators.hostRingBuffer, memoryRequirements, GetFrameIndex());
		}
		else
		{
			allocation = AllocateFromGPUMemoryRing(&gpuContext.memoryAllocators.deviceRingBuffer, memoryRequirements, GetFrameIndex());
		}
	}
	else
	{
		if (memoryType == GFX_HOST_MEMORY)
		{
			allocation = AllocateFromGPUMemoryBlocks(&gpuContext.memoryAllocators.hostBlockBuffer, memoryRequirements); // @TODO: Store these allocations so the resource can be freed.
		}
		else
		{
			allocation = AllocateFromGPUMemoryBlocks(&gpuContext.memoryAllocators.deviceBlockBuffer, memoryRequirements); // @TODO: Store these allocations so the resource can be freed.
		}
	}
	Assert(allocation);
	if (mappedPointer)
	{
		Assert(memoryType == GFX_HOST_MEMORY);
		*mappedPointer = allocation->mappedPointer;
	}
	GfxBindBufferMemory(buffer, allocation->memory, allocation->offset);
	return buffer;
}

GfxImage CreateGPUImage(u32 width, u32 height, GfxFormat format, GfxImageLayout initialLayout, GfxImageUsageFlags usage, GfxSampleCount sampleCount)
{
	auto image = GfxCreateImage(width, height, format, initialLayout, usage, sampleCount);
	auto memoryRequirements = GfxGetImageMemoryRequirements(image);
	auto allocation = AllocateFromGPUMemoryBlocks(&gpuContext.memoryAllocators.deviceBlockImage, memoryRequirements); // @TODO: Store these allocations so the resource can be freed.
	Assert(allocation);
	GfxBindImageMemory(image, allocation->memory, allocation->offset);
	//Render_API_Create_Image_View(image, format, usage)
	return image;
}

GfxCommandBuffer CreateGPUCommandBuffer(GfxCommandQueueType queue, GPUResourceLifetime lifetime)
{
	switch (queue)
	{
	case GFX_GRAPHICS_COMMAND_QUEUE:
	{
		if (lifetime == GPU_RESOURCE_LIFETIME_FRAME)
		{
			return GfxCreateCommandBuffer(gpuContext.threadLocal[GetThreadIndex()].commandPools[GetFrameIndex()].graphics);
		}
		else
		{
			// @TODO
			Abort("Creating persistent graphics command buffers not implemented yet...\n");
		}
	} break;
	case GFX_TRANSFER_COMMAND_QUEUE:
	{
		if (lifetime == GPU_RESOURCE_LIFETIME_FRAME)
		{
			return GfxCreateCommandBuffer(gpuContext.threadLocal[GetThreadIndex()].commandPools[GetFrameIndex()].transfer);
		}
		else
		{
			return GfxCreateCommandBuffer(gpuContext.commandPools.transfer);
		}
	} break;
	case GFX_COMPUTE_COMMAND_QUEUE:
	{
		// @TODO
		Abort("Creating compute command buffers not supported yet...\n");
	} break;
	default:
	{
		Abort("Invalid queue type: %d\n", queue);
	} break;
	}

	InvalidCodePath();
	return GfxCommandBuffer{};
}

void ClearGPUCommandPoolsForFrameIndex(u32 frameIndex)
{
	for (auto i = 0; i < GetWorkerThreadCount(); i++)
	{
		GfxResetCommandPool(gpuContext.threadLocal[i].commandPools[frameIndex].graphics);
		GfxResetCommandPool(gpuContext.threadLocal[i].commandPools[frameIndex].transfer);
		GfxResetCommandPool(gpuContext.threadLocal[i].commandPools[frameIndex].compute);
	}
}

void QueueGPUTransfer(GfxCommandBuffer commandBuffer)
{
	Write(&gpuContext.transferDoubleBuffer, commandBuffer);
}

void QueueGPUAsyncTransfer(GfxCommandBuffer commandBuffer, AssetLoadStatus *loadStatus = NULL)
{
	Write(&gpuContext.transferDoubleBuffer, commandBuffer);
}

GfxSemaphore SubmitGPUTransferCommands()
{
	SwitchBuffers(&gpuContext.transferDoubleBuffer);

	auto semaphore = GfxCreateSemaphore();
	GfxSubmitInfo submitInfo =
	{
		.commandBuffers = CreateArray(gpuContext.transferDoubleBuffer.readBufferElementCount, gpuContext.transferDoubleBuffer.readBuffer);
		.signalSemaphores = CreateArray(1, &semaphore);
	};
	GfxSubmitCommandBuffers(GFX_TRANSFER_COMMAND_QUEUE, submitInfo, NULL);
	return semaphore;
}
