void Create_GPU_Memory_Block_Allocator(GPU_Memory_Allocator *allocator, u32 block_size, GPU_Memory_Type memory_type) {
	allocator->type = GPU_MEMORY_BLOCK_ALLOCATOR;
	allocator->block.block_size = block_size;
	allocator->block.active_block = NULL;
	allocator->block.base_block = NULL;
	allocator->block.memory_type = memory_type;
	Platform_Create_Mutex(&allocator->block.mutex);
}

GPU_Memory_Allocation *Allocate_From_GPU_Memory_Blocks(GPU_Context *context, GPU_Memory_Block_Allocator *allocator, u32 size, u32 alignment) {
	Platform_Lock_Mutex(&allocator->mutex);
	// @TODO: Try to allocate out of the freed allocations.
	if (!allocator->active_block || allocator->active_block->frontier + size > allocator->block_size) {
		// Need to allocate a new block.
		GPU_Memory_Block *new_block = malloc(sizeof(GPU_Memory_Block)); // @TODO
		if (allocator->base_block) {
			allocator->active_block->next = new_block;
			allocator->active_block = new_block;
		} else {
			allocator->base_block = new_block;
			allocator->active_block = allocator->base_block;
		}
		if (!GPU_Allocate_Memory(context, allocator->block_size, allocator->memory_type, &new_block->memory)) {
			free(new_block); // @TODO
			return NULL;
		}
		new_block->frontier = 0;
		new_block->active_allocation_count = 0;
		new_block->next = NULL;
	}
	// Allocate out of the active block's frontier.
	GPU_Memory_Allocation *new_allocation = &allocator->active_block->active_allocations[allocator->active_block->active_allocation_count++];
	u32 allocation_start_offset = Align_U32(allocator->active_block->frontier, alignment);
	*new_allocation = (GPU_Memory_Allocation){
		.size = size,
		.offset = allocation_start_offset,
		.memory = allocator->active_block->memory,
	};
	if (allocator->memory_type & GPU_HOST_MEMORY) {
		new_allocation->mapped_pointer = GPU_Map_Memory(context, new_allocation->memory, new_allocation->size, new_allocation->offset);
	} else {
		new_allocation->mapped_pointer = NULL;
	}
	Assert(allocator->active_block->active_allocation_count < VULKAN_MAX_MEMORY_ALLOCATIONS_PER_BLOCK);
	allocator->active_block->frontier = allocation_start_offset + size;
	Assert(allocator->active_block->frontier <= allocator->block_size);
	Platform_Unlock_Mutex(&allocator->mutex);
	return new_allocation;
}

void GPU_Create_Ring_Buffer_Allocator() {
}

GPU_Memory_Allocation *Allocate_From_GPU_Memory_Ring_Buffer(GPU_Context *context, GPU_Memory_Ring_Buffer_Allocator *allocator, u32 size, u32 alignment) {
	return false;
}

// @TODO: Switch to _Generic for this stuff?
GPU_Memory_Allocation *Allocate_GPU_Memory(GPU_Context *context, GPU_Memory_Allocator *allocator, u32 size, u32 alignment) {
	switch (allocator->type) {
		case GPU_MEMORY_BLOCK_ALLOCATOR: {
			return Allocate_From_GPU_Memory_Blocks(context, &allocator->block, size, alignment);
		} break;
		case GPU_MEMORY_RING_BUFFER_ALLOCATOR: {
			return Allocate_From_GPU_Memory_Ring_Buffer(context, &allocator->ring_buffer, size, alignment);
		} break;
		default: {
			Invalid_Code_Path();
		}
	}
	return NULL;
}

bool Create_GPU_Buffer(GPU_Context *context, GPU_Memory_Allocator *allocator, GPU_Buffer_Usage_Flags buffer_usage_flags, u32 buffer_size, GPU_Buffer *buffer) {
	*buffer = GPU_Create_Buffer(context, buffer_size, buffer_usage_flags);
	GPU_Resource_Allocation_Requirements allocation_requirements = GPU_Get_Buffer_Allocation_Requirements(context, *buffer);
	GPU_Memory_Allocation *allocation = Allocate_GPU_Memory(context, allocator, allocation_requirements.size, allocation_requirements.alignment); // @TODO: Store these allocations so the resource can be freed.
	if (!allocation) {
		return false;
	}
	GPU_Bind_Buffer_Memory(context, *buffer, allocation->memory, allocation->offset);
	return true;
}

bool Create_GPU_Image(GPU_Context *context, GPU_Memory_Allocator *allocator, GPU_Image_Creation_Parameters *parameters, GPU_Image *image) {
	GPU_Resource_Allocation_Requirements allocation_requirements = GPU_Get_Image_Allocation_Requirements(context, parameters);
	GPU_Memory_Allocation *allocation = Allocate_GPU_Memory(context, allocator, allocation_requirements.size, allocation_requirements.alignment); // @TODO: Store these allocations so the resource can be freed.
	if (!allocation) {
		return false;
	}
	*image = GPU_Create_Image(context, parameters, allocation);
	return true;
}
