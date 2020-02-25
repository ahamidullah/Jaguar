namespace Renderer {

GPUMemoryBlockAllocator CreateGPUMemoryBlockAllocator(u32 blockSize, GPUMemoryType memoryType) {
	GPUMemoryBlockAllocator allocator = {};
	allocator.blockSize = blockSize;
	allocator.activeBlock = NULL;
	allocator.baseBlock = NULL;
	allocator.memoryType = memoryType;
	CreateMutex(&allocator.mutex);
	return allocator;
}

struct {
	struct {
		GPUMemoryBlockAllocator deviceBlockBuffer = CreateGPUMemoryBlockAllocator(Megabyte(8), GPU_DEVICE_MEMORY);
		GPUMemoryBlockAllocator deviceBlockImage = CreateGPUMemoryBlockAllocator(Megabyte(256), GPU_DEVICE_MEMORY);
		GPUMemoryBlockAllocator stagingBlock = CreateGPUMemoryBlockAllocator(Megabyte(256), GPU_HOST_MEMORY);
		GPUMemoryRingAllocator stagingRing; // @TODO
	} memoryAllocators;
} gpuContext;

GPUMemoryAllocation *AllocateFromGPUMemoryBlocks(GPUMemoryBlockAllocator *allocator, GPUMemoryRequirements allocationRequirements) {
	LockMutex(&allocator->mutex);
	Assert(allocationRequirements.size < allocator->blockSize);
	// @TODO: Try to allocate out of the freed allocations.
	if (!allocator->activeBlock || allocator->activeBlock->frontier + allocationRequirements.size > allocator->blockSize)
	{
		// Need to allocate a new block.
		auto newBlock = (GPUMemoryBlock *)malloc(sizeof(GPUMemoryBlock)); // @TODO
		if (!GPUAllocateMemory(allocator->blockSize, allocator->memoryType, &newBlock->memory))
		{
			free(newBlock); // @TODO
			return NULL;
		}
		newBlock->frontier = 0;
		newBlock->allocationCount = 0;
		if (allocator->memoryType & GPU_HOST_MEMORY)
		{
			newBlock->mappedPointer = GPUMapMemory(newBlock->memory, allocator->blockSize, 0);
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
	u32 allocationStartOffset = Align_U32(allocator->activeBlock->frontier, allocationRequirements.alignment);
	*newAllocation = {
		.memory = allocator->activeBlock->memory,
		.offset = allocationStartOffset,
	};
	if (allocator->memoryType & GPU_HOST_MEMORY) {
		newAllocation->mappedPointer = (char *)allocator->activeBlock->mappedPointer + allocationStartOffset;
	} else {
		newAllocation->mappedPointer = NULL;
	}
	Assert(allocator->activeBlock->allocationCount < GPU_MAX_MEMORY_ALLOCATIONS_PER_BLOCK);
	allocator->activeBlock->frontier = allocationStartOffset + allocationRequirements.size;
	Assert(allocator->activeBlock->frontier <= allocator->blockSize);
	UnlockMutex(&allocator->mutex);
	return newAllocation;
}

//void ClearGPUMemoryBlockAllocator(GPUMemoryBlockAllocator *allocator) {
void ClearGPUMemoryBlockAllocator() {
	auto allocator = &gpuContext.memoryAllocators.stagingBlock;
	LockMutex(&allocator->mutex);
	allocator->activeBlock = allocator->baseBlock;
	allocator->activeBlock->allocationCount = 0;
	allocator->activeBlock->frontier = 0;
	UnlockMutex(&allocator->mutex);
}

GPUBuffer CreateGPUBuffer(u32 size, GPUBufferUsageFlags usage_flags)
{
	auto buffer = GPUCreateBuffer(size, usage_flags);
	auto allocationRequirements = GPUGetBufferAllocationRequirements(buffer);
	auto allocation = AllocateFromGPUMemoryBlocks(&gpuContext.memoryAllocators.deviceBlockBuffer, allocationRequirements); // @TODO: Store these allocations so the resource can be freed.
	Assert(allocation);
	GPUBindBufferMemory(buffer, allocation->memory, allocation->offset);
	return buffer;
}

GPUImage CreateGPUImage(u32 width, u32 height, GPUFormat format, GPUImageLayout initial_layout, GPUImageUsageFlags usage_flags, GPUSampleCount sample_count_flags)
{
	auto image = GPUCreateImage(width, height, format, initial_layout, usage_flags, sample_count_flags);
	auto allocationRequirements = GPUGetImageAllocationRequirements(image);
	auto allocation = AllocateFromGPUMemoryBlocks(&gpuContext.memoryAllocators.deviceBlockImage, allocationRequirements); // @TODO: Store these allocations so the resource can be freed.
	Assert(allocation);
	GPUBindImageMemory(image, allocation->memory, allocation->offset);
	//Render_API_Create_Image_View(image, format, usage_flags)
	return image;
}

GPUBuffer CreateGPUStagingBuffer(u32 size, void **mappedPointer)
{
	// @TODO: Use a staging ring buffer and block-bound buffers as a fallback for overflow.
	auto buffer = GPUCreateBuffer(size, GPU_TRANSFER_SOURCE_BUFFER);
	auto allocationRequirements = GPUGetBufferAllocationRequirements(buffer);
	auto allocation = AllocateFromGPUMemoryBlocks(&gpuContext.memoryAllocators.stagingBlock, allocationRequirements);
	Assert(allocation);
	GPUBindBufferMemory(buffer, allocation->memory, allocation->offset);
	*mappedPointer = allocation->mappedPointer;
	return buffer;
}

}
