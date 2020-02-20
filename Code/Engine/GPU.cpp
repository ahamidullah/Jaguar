namespace Renderer {

GPUMemoryBlockAllocator CreateGPUMemoryBlockAllocator(u32 blockSize, GPU_Memory_Type memoryType) {
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

GPUMemoryAllocation *AllocateFromGPUMemoryBlocks(GPUMemoryBlockAllocator *allocator, GPU_Resource_Allocation_Requirements allocationRequirements) {
	LockMutex(&allocator->mutex);
	Assert(allocationRequirements.size < allocator->blockSize);
	// @TODO: Try to allocate out of the freed allocations.
	if (!allocator->activeBlock || allocator->activeBlock->frontier + allocationRequirements.size > allocator->blockSize) {
		// Need to allocate a new block.
		auto newBlock = (GPUMemoryBlock *)malloc(sizeof(GPUMemoryBlock)); // @TODO
		if (!Render_API_Allocate_Memory(allocator->blockSize, allocator->memoryType, &newBlock->memory)) {
			free(newBlock); // @TODO
			return NULL;
		}
		newBlock->frontier = 0;
		newBlock->allocationCount = 0;
		if (allocator->memoryType & GPU_HOST_MEMORY) {
			newBlock->mappedPointer = Render_API_Map_Memory(newBlock->memory, allocator->blockSize, 0);
		} else {
			newBlock->mappedPointer = NULL;
		}
		newBlock->next = NULL;
		if (allocator->baseBlock) {
			allocator->activeBlock->next = newBlock;
			allocator->activeBlock = newBlock;
		} else {
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

GPU_Buffer CreateGPUBuffer(u32 size, GPU_Buffer_Usage_Flags usage_flags) {
	GPU_Buffer buffer = Render_API_Create_Buffer(size, usage_flags);
	GPU_Resource_Allocation_Requirements allocationRequirements = Render_API_Get_Buffer_Allocation_Requirements(buffer);
	GPUMemoryAllocation *allocation = AllocateFromGPUMemoryBlocks(&gpuContext.memoryAllocators.deviceBlockBuffer, allocationRequirements); // @TODO: Store these allocations so the resource can be freed.
	Assert(allocation);
	Render_API_Bind_Buffer_Memory(buffer, allocation->memory, allocation->offset);
	return buffer;
}

GPU_Image CreateGPUImage(u32 width, u32 height, GPU_Format format, GPU_Image_Layout initial_layout, GPU_Image_Usage_Flags usage_flags, GPU_Sample_Count_Flags sample_count_flags) {
	GPU_Image image = Render_API_Create_Image(width, height, format, initial_layout, usage_flags, sample_count_flags);
	GPU_Resource_Allocation_Requirements allocationRequirements = Render_API_Get_Image_Allocation_Requirements(image);
	GPUMemoryAllocation *allocation = AllocateFromGPUMemoryBlocks(&gpuContext.memoryAllocators.deviceBlockImage, allocationRequirements); // @TODO: Store these allocations so the resource can be freed.
	Assert(allocation);
	Render_API_Bind_Image_Memory(image, allocation->memory, allocation->offset);
	//Render_API_Create_Image_View(image, format, usage_flags)
	return image;
}

GPU_Buffer CreateGPUStagingBuffer(u32 size, void **mappedPointer) {
	// @TODO: Use a staging ring buffer and block-bound buffers as a fallback for overflow.
	GPU_Buffer buffer = Render_API_Create_Buffer(size, GPU_TRANSFER_SOURCE_BUFFER);
	GPU_Resource_Allocation_Requirements allocationRequirements = Render_API_Get_Buffer_Allocation_Requirements(buffer);
	GPUMemoryAllocation *allocation = AllocateFromGPUMemoryBlocks(&gpuContext.memoryAllocators.stagingBlock, allocationRequirements);
	Assert(allocation);
	Render_API_Bind_Buffer_Memory(buffer, allocation->memory, allocation->offset);
	*mappedPointer = allocation->mappedPointer;
	return buffer;
}

}
