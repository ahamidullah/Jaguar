void Create_GPU_Memory_Block_Allocator(GPU_Memory_Block_Allocator *allocator, u32 block_size, GPU_Memory_Type memory_type) {
	allocator->block_size = block_size;
	allocator->active_block = NULL;
	allocator->base_block = NULL;
	allocator->memory_type = memory_type;
	Platform_Create_Mutex(&allocator->mutex);
}

GPU_Memory_Allocation *Allocate_From_GPU_Memory_Blocks(Render_Context *context, GPU_Memory_Block_Allocator *allocator, GPU_Resource_Allocation_Requirements allocation_requirements) {
	Platform_Lock_Mutex(&allocator->mutex);
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
	Assert(allocator->active_block->allocation_count < VULKAN_MAX_MEMORY_ALLOCATIONS_PER_BLOCK);
	allocator->active_block->frontier = allocation_start_offset + allocation_requirements.size;
	Assert(allocator->active_block->frontier <= allocator->block_size);
	Platform_Unlock_Mutex(&allocator->mutex);
	return new_allocation;
}

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

#if 0
typedef struct GPU_Subbuffer {
	GPU_Buffer buffer;
	u32 offset;
} GPU_Subbuffer;

typedef struct GPU_Staging_Buffer {
	GPU_Subbuffer subbuffer;
	void *mapped_pointer;
} GPU_Staging_Buffer;

typedef struct GPU_Host_Buffer_Memory_Block {
	GPU_Memory memory;
	u32 frontier;
	GPU_Buffer buffer;
	void *mapped_pointer;
	GPU_Host_Buffer_Memory_Block  *next;
} GPU_Host_Buffer_Memory_Block;

typedef struct GPU_Device_Buffer_Memory_Block {
	GPU_Memory memory;
	u32 frontier;
	GPU_Buffer buffer;
	GPU_Device_Buffer_Memory_Block *next;
} GPU_Device_Buffer_Memory_Block;

typedef struct GPU_Device_Image_Memory_Block {
	GPU_Memory memory;
	u32 frontier;
	GPU_Device_Image_Memory_Block *next;
} GPU_Device_Image_Memory_Block;

typedef struct GPU_Host_Buffer_Memory_Pool {
	Platform_Mutex mutex;
	u32 block_size;
	GPU_Buffer_Usage_Flags buffer_usage_flags;
	GPU_Host_Buffer_Memory_Block *active_blocks[100]; // @TODO: Hash table?
	GPU_Host_Buffer_Memory_Block *free_list;
} GPU_Buffer_Memory_Pool;

typedef struct GPU_Device_Buffer_Memory_Pool {
	Platform_Mutex mutex;
	u32 block_size;
	GPU_Buffer_Usage_Flags buffer_usage_flags;
	GPU_Device_Buffer_Memory_Block *active_blocks[100]; // @TODO: Hash table?
	GPU_Device_Buffer_Memory_Block *free_list;
} GPU_Device_Buffer_Memory_Pool;

typedef struct GPU_Device_Image_Memory_Pool {
	Platform_Mutex mutex;
	u32 block_size;
	GPU_Host_Buffer_Memory_Block *active_blocks[100]; // @TODO: Hash table?
	GPU_Host_Buffer_Memory_Block *free_list;
} GPU_Device_Image_Memory_Pool;

typedef struct GPU_Device_Memory_Pools {
	GPU_Device_Buffer_Memory_Pool vertex;
	GPU_Device_Buffer_Memory_Pool index;
	GPU_Device_Buffer_Memory_Pool uniform;
	GPU_Device_Image_Memory_Pool image;
} GPU_Device_Memory_Pools;

typedef struct GPU_Host_Memory_Pools {
	GPU_Host_Buffer_Memory_Pool vertex;
	GPU_Host_Buffer_Memory_Pool index;
	GPU_Host_Buffer_Memory_Pool uniform;
	GPU_Host_Buffer_Memory_Pool staging;
} GPU_Host_Memory_Pools;

void Create_Host_Buffer_Memory_Pool(GPU_Host_Buffer_Memory_Pool *pool, GPU_Buffer_Usage_Flags buffer_usage_flags, u32 block_size) {
	Platform_Create_Mutex(&pool->mutex);
	Set_Memory(pool->active_blocks, 0, 100);
	pool->block_size = block_size;
	pool->free_list = NULL;
	pool->buffer_usage_flags = buffer_usage_flags;
}

void Create_Device_Buffer_Memory_Pool(GPU_Device_Buffer_Memory_Pool *pool, GPU_Buffer_Usage_Flags usage_flags, u32 block_size) {
	Platform_Create_Mutex(&pool->mutex);
	Set_Memory(pool->active_blocks, 0, 100);
	pool->block_size = block_size;
	pool->free_list = NULL;
	pool->buffer_usage_flags = buffer_usage_flags;
}

void Create_Device_Image_Memory_Pool(GPU_Device_Image_Memory_Pool *pool, u32 block_size) {
	Platform_Create_Mutex(&pool->mutex);
	Set_Memory(pool->active_blocks, 0, 100);
	pool->block_size = block_size;
	pool->free_list = NULL;
}

bool Allocate_GPU_Host_Buffer_Memory(Render_Context *context, GPU_Host_Buffer_Memory_Pool *pool, u32 size, u32 memory_tag, GPU_Subbuffer *subbuffer, void **mapped_pointer) {
	Platform_Lock_Mutex(&allocator->mutex);
	if (pool->active_blocks[memory_tag] == NULL || pool->active_blocks[memory_tag].frontier + size > pool->block_size) {
		GPU_Host_Buffer_Memory_Block *new_block;
		Atomic_Pop_From_Front_Of_List(pool->free_list, new_block);
		if (!new_block) {
			new_block = malloc(sizeof(GPU_Host_Buffer_Memory_Block)); // @TODO
		}
		Assert(GPU_Allocate_Memory(context, size, GPU_HOST_MEMORY, &new_block->memory));
		new_block->buffer = GPU_Create_Buffer(context, pool->buffer_usage_flags, size);
		new_block->frontier = 0;
		new_block->mapped_pointer = GPU_Map_Memory(context, new_block->memory, size, u32 offset);
		new_block->next = pool->active_blocks[memory_tag];
		pool->active_blocks[memory_tag] = new_block;
	}
	*mapped_pointer = (char *)pool->active_blocks[memory_tag].mapped_pointer + pool->active_blocks[memory_tag].frontier;
	subbuffer->buffer = pool->active_blocks[memory_tag].buffer;
	subbuffer->offset = pool->active_blocks[memory_tag].frontier;
	pool->active_blocks[memory_tag].frontier += size;
	Platform_Unlock_Mutex(&allocator->mutex);
}

bool Allocate_GPU_Device_Buffer_Memory(GPU_Device_Buffer_Memory_Pool *pool, u32 size, u32 memory_tag, GPU_Subbuffer *subbuffer) {
}

bool Allocate_GPU_Device_Image_Memory(GPU_Device_Image_Memory_Pool *pool, u32 size, u32 memory_tag, GPU_Memory *memory, u32 *offset) {
}


void Create_GPU_Buffer_Block_Allocator(GPU_Buffer_Block_Allocator *allocator, GPU_Buffer_Usage_Flags buffer_usage_flags, u32 block_size, GPU_Memory_Type memory_type) {
	Platform_Create_Mutex(&allocator->mutex);
	allocator->block_size = block_size;
	allocator->memory_type = memory_type;
	allocator->buffer_usage_flags = buffer_usage_flags;
	allocator->base_block = NULL;
	allocator->active_block = NULL;
}

void Create_GPU_Image_Block_Allocator(GPU_Image_Block_Allocator *allocator, u32 block_size, GPU_Memory_Type memory_type) {
	Platform_Create_Mutex(&allocator->mutex);
	allocator->block_size = block_size;
	allocator->memory_type = memory_type;
	allocator->base_block = NULL;
	allocator->active_block = NULL;
}

void Create_GPU_Buffer_Ring_Allocator(GPU_Buffer_Ring_Allocator *allocator, GPU_Buffer_Usage_Flags usage_flags, u32 size) {
	allocator->size = size;
	allocator->read_index = 0;
	allocator->write_index = 0;
	allocator->mapped_pointer = NULL;
}

#define GPU_Block_Allocator_Push_Size(context, allocator, size, alignment, output_new_block, output_start_offset) \
	do { \
		if (!allocator->active_block || allocator->active_block->frontier + size > allocator->block_size) { \
			/* Need to allocate a new block. */ \
			typeof(allocator->active_block) new_block = malloc(sizeof(typeof(allocator->active_block))); /* TODO */ \
			if (!GPU_Allocate_Memory(&context->api_context, allocator->block_size, allocator->memory_type, &new_block->memory)) { \
				free(new_block); /* @TODO */ \
				Assert(0); /* TODO */ \
			} \
			new_block->frontier = 0; \
			new_block->allocation_count = 0; \
			new_block->next = NULL; \
			if (allocator->base_block) { \
				allocator->active_block->next = new_block; \
				allocator->active_block = new_block; \
			} else { \
				allocator->base_block = new_block; \
				allocator->active_block = allocator->base_block; \
			} \
			output_new_block = new_block; \
		} else { \
			output_new_block = NULL; \
		} \
		u32 aligned_frontier = Align_U32(allocator->active_block->frontier, alignment); \
		output_start_offset = &allocator->active_block->allocations[allocator->active_block->allocation_count++]; \
		*output_start_offset = aligned_frontier; \
		allocator->active_block->frontier = aligned_frontier + size; \
		Assert(allocator->active_block->allocation_count < VULKAN_MAX_MEMORY_ALLOCATIONS_PER_BLOCK); \
		Assert(allocator->active_block->frontier <= allocator->block_size); \
	} while (0)

GPU_Subbuffer GPU_Buffer_Block_Allocator_Push_Size(Render_Context *context, GPU_Buffer_Block_Allocator *allocator, u32 size) {
	GPU_Buffer_Memory_Block *new_buffer_block;
	u32 *subbuffer_start_offset;
	GPU_Block_Allocator_Push_Size(context, allocator, size, 1, new_buffer_block, subbuffer_start_offset);
	if (new_buffer_block) {
		new_buffer_block->buffer = GPU_Create_Buffer(&context->api_context, allocator->buffer_usage_flags, size);
		new_buffer_block->mapped_pointer = NULL;
	}
	if (new_buffer_block && (allocator->memory_type & GPU_HOST_MEMORY)) {
		new_buffer_block->mapped_pointer = GPU_Map_Memory(&context->api_context, new_buffer_block->memory, allocator->block_size, 0);
	}
	return (GPU_Subbuffer){
		.buffer = allocator->active_block->buffer,
		.offset = subbuffer_start_offset,
	};
}

GPU_Subbuffer GPU_Host_Buffer_Block_Allocator_Push_Size(Render_Context *context, GPU_Buffer_Block_Allocator *allocator, u32 size, void **mapped_pointer) {
	GPU_Subbuffer subbuffer = GPU_Buffer_Block_Allocator_Push_Size(context, allocator, size);
	*mapped_pointer = (char *)allocator->active_block->mapped_pointer + *subbuffer.offset;
	return subbuffer;
}

GPU_Subbuffer GPU_Device_Buffer_Block_Allocator_Push_Size(Render_Context *context, GPU_Buffer_Block_Allocator *allocator, u32 size) {
	GPU_Subbuffer subbuffer = GPU_Buffer_Block_Allocator_Push_Size(context, allocator, size);
	return subbuffer;
}

GPU_Image_Allocation GPU_Device_Image_Block_Allocator_Push_Size(Render_Context *context, GPU_Image_Block_Allocator *allocator, u32 size, u32 alignment) {
	GPU_Image_Memory_Block *new_image_block;
	u32 *allocation_start_offset;
	GPU_Block_Allocator_Push_Size(context, allocator, size, alignment, new_image_block, allocation_start_offset);
	return (GPU_Image_Allocation){
		.memory = allocator->active_block->memory,
		.offset = allocation_start_offset,
	};
}

GPU_Subbuffer Create_GPU_Host_Vertex_Subbuffer(Render_Context *context, u32 size, void **mapped_pointer) {
	GPU_Buffer_Block_Allocator *allocator = &context->gpu_memory_allocators.host_vertex_block;
	Platform_Lock_Mutex(&allocator->mutex);
	GPU_Subbuffer subbuffer = GPU_Host_Buffer_Block_Allocator_Push_Size(context, allocator, size, mapped_pointer);
	Platform_Unlock_Mutex(&allocator->mutex);
	return subbuffer;
}

GPU_Subbuffer Create_GPU_Host_Index_Subbuffer(Render_Context *context, u32 size, void **mapped_pointer) {
	GPU_Buffer_Block_Allocator *allocator = &context->gpu_memory_allocators.host_index_block;
	Platform_Lock_Mutex(&allocator->mutex);
	GPU_Subbuffer subbuffer = GPU_Host_Buffer_Block_Allocator_Push_Size(context, allocator, size, mapped_pointer);
	Platform_Unlock_Mutex(&allocator->mutex);
	return subbuffer;
}

GPU_Subbuffer Create_GPU_Host_Uniform_Subbuffer(Render_Context *context, u32 size, void **mapped_pointer) {
	GPU_Buffer_Block_Allocator *allocator = &context->gpu_memory_allocators.host_uniform_block;
	Platform_Lock_Mutex(&allocator->mutex);
	GPU_Subbuffer subbuffer = GPU_Host_Buffer_Block_Allocator_Push_Size(context, allocator, size, mapped_pointer);
	Platform_Unlock_Mutex(&allocator->mutex);
	return subbuffer;
}

GPU_Subbuffer Create_GPU_Device_Vertex_Subbuffer(Render_Context *context, u32 size) {
	GPU_Buffer_Block_Allocator *allocator = &context->gpu_memory_allocators.device_vertex_block;
	Platform_Lock_Mutex(&allocator->mutex);
	GPU_Subbuffer subbuffer = GPU_Device_Buffer_Block_Allocator_Push_Size(context, allocator, size);
	Platform_Unlock_Mutex(&allocator->mutex);
	return subbuffer;
}

GPU_Subbuffer Create_GPU_Device_Index_Subbuffer(Render_Context *context, u32 size) {
	GPU_Buffer_Block_Allocator *allocator = &context->gpu_memory_allocators.device_index_block;
	Platform_Lock_Mutex(&allocator->mutex);
	GPU_Subbuffer subbuffer = GPU_Device_Buffer_Block_Allocator_Push_Size(context, allocator, size);
	Platform_Unlock_Mutex(&allocator->mutex);
	return subbuffer;
}

GPU_Subbuffer Create_GPU_Device_Uniform_Subbuffer(Render_Context *context, u32 size) {
	GPU_Buffer_Block_Allocator *allocator = &context->gpu_memory_allocators.device_uniform_block;
	Platform_Lock_Mutex(&allocator->mutex);
	GPU_Subbuffer subbuffer = GPU_Device_Buffer_Block_Allocator_Push_Size(context, allocator, size);
	Platform_Unlock_Mutex(&allocator->mutex);
	return subbuffer;
}

GPU_Image Create_GPU_Device_Image(Render_Context *context, GPU_Image_Creation_Parameters *parameters) {
	GPU_Image_Block_Allocator *allocator = &context->gpu_memory_allocators.device_image_block;
	GPU_Resource_Allocation_Requirements allocation_requirements = GPU_Get_Image_Allocation_Requirements(&context->api_context, parameters);
	Platform_Lock_Mutex(&allocator->mutex);
	GPU_Image_Allocation image_allocation = GPU_Device_Image_Block_Allocator_Push_Size(context, allocator, allocation_requirements.size, allocation_requirements.alignment);
	Platform_Unlock_Mutex(&allocator->mutex);
	return GPU_Create_Image(&context->api_context, parameters, image_allocation);
}
#endif

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

GPU_Indexed_Geometry Queue_Indexed_Geometry_Upload_To_GPU(Render_Context *context, u32 vertices_size, u32 indices_size, GPU_Staging_Buffer staging_buffer) {
	GPU_Buffer vertex_buffer = Create_GPU_Device_Buffer(context, vertices_size, GPU_VERTEX_BUFFER | GPU_TRANSFER_DESTINATION_BUFFER);
	GPU_Buffer index_buffer = Create_GPU_Device_Buffer(context, indices_size, GPU_INDEX_BUFFER | GPU_TRANSFER_DESTINATION_BUFFER);
	GPU_Command_Buffer command_buffer = Render_API_Create_Command_Buffer(&context->api_context, context->thread_local_contexts[thread_index].command_pools[context->current_frame_index]);
	Render_API_Record_Copy_Buffer_Command(&context->api_context, command_buffer, vertices_size, staging_buffer.buffer, vertex_buffer, staging_buffer.offset, 0);
	Render_API_Record_Copy_Buffer_Command(&context->api_context, command_buffer, indices_size, staging_buffer.buffer, index_buffer, staging_buffer.offset + vertices_size, 0);
	Render_API_End_Command_Buffer(&context->api_context, command_buffer);
	return (GPU_Indexed_Geometry){
		.vertex_buffer = vertex_buffer,
		.index_buffer = index_buffer,
	};
#if 0
	GPU_Buffer staging_buffer;
	if (!Stage_GPU_Upload(context, vertices_size, vertices, context->frame_number) || !Stage_GPU_Upload(context, indices_size, indices, context->frame_number)) {
		Abort("Ran out of staging memory\n"); // @TODO
	}
	GPU_Buffer vertex_buffer, index_buffer;
	if (!Create_GPU_Buffer() || !Create_GPU_Buffer()) {
		Abort("Unable to allocate geometry buffer\n"); // @TODO
	}
	Copy_Memory(vertices, staging_buffer.mapped_pointer, vertices_size);
	Copy_Memory(indices, ((char *)staging_buffer.mapped_pointer) + vertices_size, indices_size);
	//GPU_Memory_Allocation *gpu_memory = Allocate_From_GPU_Memory_Blocks(context, &context->memory.device_block_allocator, size, );
	GPU_Command_Buffer command_buffer = GPU_Create_Command_Buffer(context->thread_local[thread_index].command_pools);
	GPU_Record_Copy_Buffer_Command(command_buffer, staging_buffer, vertex_buffer, vertices_size, 0, 0);
	GPU_Record_Copy_Buffer_Command(command_buffer, staging_buffer, index_buffer, indices_size, vertices_size, 0);
	GPU_End_Command_Buffer(command_buffer);
	Atomic_Write_To_Ring_Buffer(asset_upload_command_buffers, command_buffer);
	return (GPU_Mesh){
		.vertex_buffer = vertex_buffer,
		.index_buffer = index_buffer,
	};
#endif

	//GPU_Create_Command_List(vulkan_context.thread_local[thread_index].command_pools, &command_list);
	//GPU_End_Command_List(command_list);
	//Atomic_Write_To_Ring_Buffer(asset_upload_command_buffers, command_list);
	//return (GPU_Mesh){
		//.memory = gpu_memory,
		//.indices_offset = vertices_size,
	//};
}

typedef u32 GPU_Texture_ID;

GPU_Texture_ID Queue_Texture_Upload_To_GPU(Render_Context *context, u8 *pixels, s32 texture_width, s32 texture_height) {
	// @TODO: Load texture directly into staging memory.
	void *staging_memory;
	u32 texture_byte_size = sizeof(u32) * texture_width * texture_height;
	GPU_Staging_Buffer staging_buffer = Create_GPU_Staging_Buffer(context, texture_byte_size, &staging_memory);
	Copy_Memory(pixels, staging_memory, texture_byte_size);
	/*
	GPU_Image_Creation_Parameters texture_creation_parameters = {
		.width = texture_width,
		.height = texture_height,
		.format = GPU_FORMAT_R8G8B8A8_UNORM,
		.initial_layout = GPU_IMAGE_LAYOUT_UNDEFINED,
		.usage_flags = GPU_IMAGE_USAGE_TRANSFER_DST | GPU_IMAGE_USAGE_SAMPLED,
		.sample_count_flags = GPU_SAMPLE_COUNT_1,
	};
	*/
	GPU_Image image = Create_GPU_Device_Image(context, texture_width, texture_height, GPU_FORMAT_R8G8B8A8_UNORM, GPU_IMAGE_LAYOUT_UNDEFINED, GPU_IMAGE_USAGE_TRANSFER_DST | GPU_IMAGE_USAGE_SAMPLED, GPU_SAMPLE_COUNT_1);
	GPU_Command_Buffer command_buffer = Render_API_Create_Command_Buffer(&context->api_context, context->thread_local_contexts[thread_index].command_pools[context->current_frame_index]);
	Render_API_Transition_Image_Layout(&context->api_context, command_buffer, image, GPU_FORMAT_R8G8B8A8_UNORM, GPU_IMAGE_LAYOUT_UNDEFINED, GPU_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	Render_API_Record_Copy_Buffer_To_Image_Command(&context->api_context, command_buffer, staging_buffer.buffer, image, texture_width, texture_height);
	Render_API_Transition_Image_Layout(&context->api_context, command_buffer, image, GPU_FORMAT_R8G8B8A8_UNORM, GPU_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, GPU_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	Render_API_End_Command_Buffer(&context->api_context, command_buffer);
	Render_API_Submit_Command_Buffers(&context->api_context, 1, &command_buffer, GPU_GRAPHICS_COMMAND_QUEUE);
	return 0;
}
