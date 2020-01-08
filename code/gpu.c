void Create_GPU_Memory_Block_Allocator(GPU_Memory_Block_Allocator *allocator, u32 block_size, GPU_Memory_Type memory_type) {
	allocator->block_size = block_size;
	allocator->active_block = NULL;
	allocator->base_block = NULL;
	allocator->memory_type = memory_type;
	Platform_Create_Mutex(&allocator->mutex);
}

GPU_Memory_Allocation *Allocate_From_GPU_Memory_Blocks(Render_Context *context, GPU_Memory_Block_Allocator *allocator, GPU_Resource_Allocation_Requirements allocation_requirements) {
	Platform_Lock_Mutex(&allocator->mutex);
	Assert(allocation_requirements.size < allocator->block_size);
	// @TODO: Try to allocate out of the freed allocations.
	if (!allocator->active_block || allocator->active_block->frontier + allocation_requirements.size > allocator->block_size) {
		// Need to allocate a new block.
		GPU_Memory_Block *new_block = malloc(sizeof(GPU_Memory_Block)); // @TODO
		if (!Render_API_Allocate_Memory(&context->api_context, allocator->block_size, allocator->memory_type, &new_block->memory)) {
			free(new_block); // @TODO
			return NULL;
		}
		new_block->frontier = 0;
		new_block->allocation_count = 0;
		if (allocator->memory_type & GPU_HOST_MEMORY) {
			new_block->mapped_pointer = Render_API_Map_Memory(&context->api_context, new_block->memory, allocator->block_size, 0);
		} else {
			new_block->mapped_pointer = NULL;
		}
		new_block->next = NULL;
		if (allocator->base_block) {
			allocator->active_block->next = new_block;
			allocator->active_block = new_block;
		} else {
			allocator->base_block = new_block;
			allocator->active_block = allocator->base_block;
		}
	}
	// Allocate out of the active block's frontier.
	GPU_Memory_Allocation *new_allocation = &allocator->active_block->allocations[allocator->active_block->allocation_count++];
	u32 allocation_start_offset = Align_U32(allocator->active_block->frontier, allocation_requirements.alignment);
	*new_allocation = (GPU_Memory_Allocation){
		.offset = allocation_start_offset,
		.memory = allocator->active_block->memory,
	};
	if (allocator->memory_type & GPU_HOST_MEMORY) {
		new_allocation->mapped_pointer = (char *)allocator->active_block->mapped_pointer + allocation_start_offset;
	} else {
		new_allocation->mapped_pointer = NULL;
	}
	Assert(allocator->active_block->allocation_count < GPU_MAX_MEMORY_ALLOCATIONS_PER_BLOCK);
	allocator->active_block->frontier = allocation_start_offset + allocation_requirements.size;
	Assert(allocator->active_block->frontier <= allocator->block_size);
	Platform_Unlock_Mutex(&allocator->mutex);
	return new_allocation;
}

void Clear_GPU_Memory_Block_Allocator(GPU_Memory_Block_Allocator *allocator) {
	Platform_Lock_Mutex(&allocator->mutex);
	allocator->active_block = allocator->base_block;
	allocator->active_block->allocation_count = 0;
	allocator->active_block->frontier = 0;
	Platform_Unlock_Mutex(&allocator->mutex);
}

#if 0
void Create_GPU_Memory_Ring_Allocator(GPU_Memory_Ring_Allocator *allocator, u32 size, GPU_Memory_Type memory_type) {
	allocator->memory_type = memory_type;
	if (!Render_API_Allocate_Memory(&context->api_context, size, allocator->memory_type, &allocator->memory)) {
		Assert(0); // @TODO;
	}
}

GPU_Memory_Allocation *Allocate_From_GPU_Memory_Ring(Render_Context *context, GPU_Memory_Ring_Allocator *allocator, GPU_Resource_Allocation_Requirements allocation_requirements) {
}
#endif

typedef struct GPU_Buffer_Creation_Paramters {
	u32 size;
	GPU_Buffer_Usage_Flags usage_flags;
} GPU_Buffer_Creation_Paramters;

GPU_Buffer Create_GPU_Device_Buffer(Render_Context *context, u32 size, GPU_Buffer_Usage_Flags usage_flags) {
	GPU_Buffer buffer = Render_API_Create_Buffer(&context->api_context, size, usage_flags);
	GPU_Resource_Allocation_Requirements allocation_requirements = Render_API_Get_Buffer_Allocation_Requirements(&context->api_context, buffer);
	GPU_Memory_Allocation *allocation = Allocate_From_GPU_Memory_Blocks(context, &context->gpu_memory_allocators.device_block_buffer, allocation_requirements); // @TODO: Store these allocations so the resource can be freed.
	Assert(allocation);
	Render_API_Bind_Buffer_Memory(&context->api_context, buffer, allocation->memory, allocation->offset);
	return buffer;
}

GPU_Image Create_GPU_Device_Image(Render_Context *context, u32 width, u32 height, GPU_Format format, GPU_Image_Layout initial_layout, GPU_Image_Usage_Flags usage_flags, GPU_Sample_Count_Flags sample_count_flags) {
	GPU_Image image = Render_API_Create_Image(&context->api_context, width, height, format, initial_layout, usage_flags, sample_count_flags);
	GPU_Resource_Allocation_Requirements allocation_requirements = Render_API_Get_Image_Allocation_Requirements(&context->api_context, image);
	GPU_Memory_Allocation *allocation = Allocate_From_GPU_Memory_Blocks(context, &context->gpu_memory_allocators.device_block_image, allocation_requirements); // @TODO: Store these allocations so the resource can be freed.
	Assert(allocation);
	Render_API_Bind_Image_Memory(&context->api_context, image, allocation->memory, allocation->offset);
	//Render_API_Create_Image_View(&context->api_context, image, format, usage_flags)
	return image;
}

// @TODO: Get rid of this and just use a GPU_Buffer.
typedef struct GPU_Staging_Buffer {
	GPU_Buffer buffer;
	u32 offset;
} GPU_Staging_Buffer;

GPU_Staging_Buffer Create_GPU_Staging_Buffer(Render_Context *context, u32 size, void **mapped_pointer) {
	// @TODO: Use a staging ring buffer and block-bound buffers as a fallback for overflow.
	GPU_Buffer buffer = Render_API_Create_Buffer(&context->api_context, size, GPU_TRANSFER_SOURCE_BUFFER);
	GPU_Resource_Allocation_Requirements allocation_requirements = Render_API_Get_Buffer_Allocation_Requirements(&context->api_context, buffer);
	GPU_Memory_Allocation *allocation = Allocate_From_GPU_Memory_Blocks(context, &context->gpu_memory_allocators.staging_block, allocation_requirements);
	Assert(allocation);
	Render_API_Bind_Buffer_Memory(&context->api_context, buffer, allocation->memory, allocation->offset);
	*mapped_pointer = allocation->mapped_pointer;
	return (GPU_Staging_Buffer){
		.buffer = buffer,
		.offset = 0,
	};
}

typedef u32 GPU_Texture_ID;
