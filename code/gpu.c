#if 0
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
		if (!GPU_Allocate_Memory(context, allocator->block_size, allocator->memory_type, &new_block->memory)) {
			free(new_block); // @TODO
			return NULL;
		}
		new_block->frontier = 0;
		new_block->active_allocation_count = 0;
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

bool Create_GPU_Buffer(GPU_Context *context, GPU_Memory_Block_Allocator *allocator, GPU_Buffer_Usage_Flags buffer_usage_flags, u32 buffer_size, GPU_Buffer *buffer) {
	*buffer = GPU_Create_Buffer(context, buffer_size, buffer_usage_flags);
	GPU_Resource_Allocation_Requirements allocation_requirements = GPU_Get_Buffer_Allocation_Requirements(context, *buffer);
	GPU_Memory_Allocation *allocation = Allocate_From_GPU_Memory_Blocks(context, allocator, allocation_requirements.size, allocation_requirements.alignment); // @TODO: Store these allocations so the resource can be freed.
	if (!allocation) {
		return false;
	}
	GPU_Bind_Buffer_Memory(context, *buffer, allocation->memory, allocation->offset);
	return true;
}

bool Create_GPU_Image(GPU_Context *context, GPU_Memory_Block_Allocator *allocator, GPU_Image_Creation_Parameters *parameters, GPU_Image *image) {
	GPU_Resource_Allocation_Requirements allocation_requirements = GPU_Get_Image_Allocation_Requirements(context, parameters);
	GPU_Memory_Allocation *allocation = Allocate_From_GPU_Memory_Blocks(context, allocator, allocation_requirements.size, allocation_requirements.alignment); // @TODO: Store these allocations so the resource can be freed.
	if (!allocation) {
		return false;
	}
	*image = GPU_Create_Image(context, parameters, allocation);
	return true;
}
#endif

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

#endif

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
			if (!GPU_Allocate_Memory(&context->gpu_context, allocator->block_size, allocator->memory_type, &new_block->memory)) { \
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
		new_buffer_block->buffer = GPU_Create_Buffer(&context->gpu_context, allocator->buffer_usage_flags, size);
		new_buffer_block->mapped_pointer = NULL;
	}
	if (new_buffer_block && (allocator->memory_type & GPU_HOST_MEMORY)) {
		new_buffer_block->mapped_pointer = GPU_Map_Memory(&context->gpu_context, new_buffer_block->memory, allocator->block_size, 0);
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
	GPU_Resource_Allocation_Requirements allocation_requirements = GPU_Get_Image_Allocation_Requirements(&context->gpu_context, parameters);
	Platform_Lock_Mutex(&allocator->mutex);
	GPU_Image_Allocation image_allocation = GPU_Device_Image_Block_Allocator_Push_Size(context, allocator, allocation_requirements.size, allocation_requirements.alignment);
	Platform_Unlock_Mutex(&allocator->mutex);
	return GPU_Create_Image(&context->gpu_context, parameters, image_allocation);
}

GPU_Subbuffer Create_GPU_Staging_Subbuffer(Render_Context *context, u32 size, void **mapped_pointer) {
	GPU_Buffer_Block_Allocator *allocator = &context->gpu_memory_allocators.host_staging_block;
	Platform_Lock_Mutex(&allocator->mutex);
	GPU_Subbuffer subbuffer = GPU_Host_Buffer_Block_Allocator_Push_Size(context, allocator, size, mapped_pointer);
	Platform_Unlock_Mutex(&allocator->mutex);
	return subbuffer;
}

GPU_Indexed_Geometry Upload_Staged_Indexed_Geometry_To_GPU(Render_Context *context, u32 vertices_size, u32 indices_size, GPU_Subbuffer staging_subbuffer) {
	GPU_Subbuffer vertex_subbuffer = Create_GPU_Device_Vertex_Subbuffer(context, vertices_size);
	GPU_Subbuffer index_subbuffer = Create_GPU_Device_Index_Subbuffer(context, indices_size);
	GPU_Command_Buffer command_buffer = GPU_Create_Command_Buffer(&context->gpu_context, context->gpu_context.thread_local[thread_index].command_pools[context->current_frame_index]);
	GPU_Record_Copy_Buffer_Commands(&context->gpu_context, command_buffer, 1, &vertices_size, staging_subbuffer.buffer, vertex_subbuffer.buffer, staging_subbuffer.offset, vertex_subbuffer.offset);
	GPU_Record_Copy_Buffer_Commands(&context->gpu_context, command_buffer, 1, &indices_size, staging_subbuffer.buffer, index_subbuffer.buffer, staging_subbuffer.offset + vertices_size, index_subbuffer.offset);
	GPU_End_Command_Buffer(&context->gpu_context, command_buffer);
	return (GPU_Indexed_Geometry){
		.vertex_subbuffer = vertex_subbuffer,
		.index_subbuffer = index_subbuffer,
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
