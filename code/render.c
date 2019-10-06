bool Allocate_From_Render_Memory_Blocks(Render_Memory_Block_Allocator *allocator, u32 size, u32 alignment, Render_Memory_Allocation **allocation) {
	Platform_Lock_Mutex(&allocator->mutex);
	// @TODO: Try to allocate out of the freed allocations.
	if (!allocator->active_block || allocator->active_block->frontier + size > allocator->block_size) {
		// Need to allocate a new block.
		Render_Memory_Block *new_block = malloc(sizeof(Render_Memory_Block)); // @TODO
		if (allocator->base_block) {
			allocator->active_block->next = new_block;
			allocator->active_block = new_block;
		} else {
			allocator->base_block = new_block;
			allocator->active_block = allocator->base_block;
		}
		if (!GPU_Allocate_Memory(allocator->block_size, allocator->memory_type, &new_block->memory)) {
			free(new_block); // @TODO
			return false;
		}
		new_block->frontier = 0;
		new_block->active_allocation_count = 0;
		new_block->next = NULL;
	}
	// Allocate out of the active block's frontier.
	// @TODO: Handle alignment.
	Render_Memory_Allocation *new_allocation = &allocator->active_block->active_allocations[allocator->active_block->active_allocation_count++];
	*new_allocation = (Render_Memory_Allocation){
		.size = size,
		.offset = allocator->active_block->frontier,
		.memory = allocator->active_block->memory,
	};
	new_allocation->mapped_pointer = GPU_Map_Memory(new_allocation->memory, new_allocation->size, new_allocation->offset);
	Assert(allocator->active_block->active_allocation_count < VULKAN_MAX_MEMORY_ALLOCATIONS_PER_BLOCK);
	allocator->active_block->frontier += size;
	Assert(allocator->active_block->frontier <= allocator->block_size);
	*allocation = new_allocation;
	Platform_Unlock_Mutex(&allocator->mutex);
	return true;
}

void Create_Render_Memory_Block_Allocator(Render_Memory_Block_Allocator *allocator, u32 block_size, GPU_Memory_Type memory_type) {
	allocator->block_size = block_size;
	allocator->active_block = NULL;
	allocator->base_block = NULL;
	allocator->memory_type = memory_type;
	Platform_Create_Mutex(&allocator->mutex);
}

void Create_Render_Ring_Buffer_Allocator() {
}

void Allocate_From_Render_Ring_Buffer(Render_Ring_Buffer_Allocator *allocator, u32 size) {
}

typedef struct Render_Buffer {
	GPU_Buffer gpu_buffer;
	Render_Memory_Allocation *allocation;
} Render_Buffer;

u8 Create_Render_Buffer(Render_Context *context, GPU_Buffer_Usage_Flags buffer_usage_flags, u32 buffer_size, Render_Buffer *buffer) {
	buffer->gpu_buffer = GPU_Create_Buffer(buffer_size, buffer_usage_flags);
	u32 allocation_size, allocation_alignment;
	GPU_Get_Buffer_Allocation_Requirements(buffer->gpu_buffer, &allocation_size, &allocation_alignment);
	if (!Allocate_From_Render_Memory_Blocks(&context->memory.device_block_allocator, allocation_size, allocation_alignment, &buffer->allocation)) {
		return false;
	}
	GPU_Bind_Buffer_Memory(buffer->gpu_buffer, buffer->allocation->memory, buffer->allocation->offset);
	return true;
}

#define RANDOM_COLOR_TABLE_LENGTH 1024
V3 random_color_table[RANDOM_COLOR_TABLE_LENGTH];

void Add_Debug_Render_Object(Render_Context *context, void *vertices, u32 vertex_count, size_t sizeof_vertex, u32 *indices, u32 index_count, V4 color, Render_Primitive primitive) {
	Debug_Render_Object *object = &context->debug_render_objects[context->debug_render_object_count++];
	*object = (Debug_Render_Object){
		.index_count = index_count,
		.color = color,
		.render_primitive = primitive,
	};
	//vulkan_push_debug_vertices(vertices, vertex_count, sizeof_vertex, indices, index_count, &object->vertex_offset, &object->first_index);
	Assert(context->debug_render_object_count < MAX_DEBUG_RENDER_OBJECTS);
}

void Draw_Wire_Sphere(Render_Context *context, V3 center, f32 radius, V4 color) {
	const u32 sector_count = 30;
	const u32 stack_count  = 15;
	f32 sector_step = 2.0f * M_PI / (f32)sector_count;
	f32 stack_step = M_PI / (f32)stack_count;
	f32 sector_angle, stack_angle;
	f32 x, y, z, xy;
	Vertex1P vertices[((sector_count + 1) * (stack_count + 1))];
	u32 vertex_count = 0;
	for (u32 i = 0; i <= stack_count; ++i) {
		stack_angle = M_PI / 2 - i * stack_step; // From pi/2 to -pi/2.
		xy = radius * cosf(stack_angle); // r * cos(u)
		z = radius * sinf(stack_angle); // r * sin(u)
		for (u32 j = 0; j <= sector_count; ++j) {
			sector_angle = j * sector_step; // From 0 to 2pi.
			x = xy * cosf(sector_angle); // r * cos(u) * cos(v)
			y = xy * sinf(sector_angle); // r * cos(u) * sin(v)
			vertices[vertex_count++].position = add_v3((V3){x, y, z}, center);
		}
	}
	u32 indices[4 * stack_count * sector_count];
	u32 index_count = 0;
	u32 k1, k2;
	for (u32 i = 0; i < stack_count; ++i) {
		k1 = i * (sector_count + 1);
		k2 = k1 + sector_count + 1;
		for (u32 j = 0; j < sector_count; ++j, ++k1, ++k2) {
			indices[index_count++] = k1;
			indices[index_count++] = k2;
			if (i != 0) {
				indices[index_count++] = k1;
				indices[index_count++] = k1 + 1;
			}
		}
	}
	Add_Debug_Render_Object(context, vertices, vertex_count, sizeof(vertices[0]), indices, index_count, color, LINE_PRIMITIVE);
}

V3 random_color() {
	float r = rand() / (float)RAND_MAX;
	float g = rand() / (float)RAND_MAX;
	float b = rand() / (float)RAND_MAX;
	return (V3){r, g, b};
}

#define MAX_VISIBLE_ENTITY_MESHES MAX_ENTITY_MESHES

void Frustum_Cull_Meshes(Camera *camera, Bounding_Sphere *mesh_bounding_spheres, u32 mesh_bounding_sphere_count, f32 focal_length, f32 aspect_ratio, u32 *visible_meshes, u32 *visible_mesh_count) {
	f32 half_near_plane_height = focal_length * tan(camera->field_of_view / 2.0f);
	f32 half_near_plane_width = focal_length * tan(camera->field_of_view / 2.0f) * aspect_ratio;
	V3 center_of_near_plane = scale_v3(focal_length, camera->forward);
	enum {
		RIGHT,
		LEFT,
		TOP,
		BOTTOM,
		FRUSTUM_PLANE_COUNT,
	};
	V3 frustum_plane_normals[] = {
		[RIGHT] = cross_product(normalize(add_v3(center_of_near_plane, scale_v3(half_near_plane_width, camera->side))), camera->up),
		[LEFT] = cross_product(camera->up, normalize(subtract_v3(center_of_near_plane, scale_v3(half_near_plane_width, camera->side)))),
		[TOP] = cross_product(camera->side, normalize(add_v3(center_of_near_plane, scale_v3(half_near_plane_height, camera->up)))),
		[BOTTOM] = cross_product(normalize(subtract_v3(center_of_near_plane, scale_v3(half_near_plane_height, camera->up))), camera->side),
	};
	for (u32 i = 0; i < mesh_bounding_sphere_count; i++) {
		//Draw_Wire_Sphere(mesh_bounding_spheres[i].center, mesh_bounding_spheres[i].radius, v3_to_v4(random_color_table[i % RANDOM_COLOR_TABLE_LENGTH], 1.0f));
		if ((dot_product(frustum_plane_normals[RIGHT], mesh_bounding_spheres[i].center) - dot_product(frustum_plane_normals[RIGHT], camera->position) <= mesh_bounding_spheres[i].radius)
		 && (dot_product(frustum_plane_normals[LEFT], mesh_bounding_spheres[i].center) - dot_product(frustum_plane_normals[LEFT], camera->position) <= mesh_bounding_spheres[i].radius)
		 && (dot_product(frustum_plane_normals[TOP], mesh_bounding_spheres[i].center) - dot_product(frustum_plane_normals[TOP], camera->position) <= mesh_bounding_spheres[i].radius)
		 && (dot_product(frustum_plane_normals[BOTTOM], mesh_bounding_spheres[i].center) - dot_product(frustum_plane_normals[BOTTOM], camera->position) <= mesh_bounding_spheres[i].radius)) {
		 	Assert(*visible_mesh_count < MAX_VISIBLE_ENTITY_MESHES);
			visible_meshes[(*visible_mesh_count)++] = i;
		}
	}
}

#define MAX_ASSET_UPLOAD_COMMAND_BUFFERS 100

typedef struct Asset_Upload_Command_Buffers {
	GPU_Command_List elements[MAX_ASSET_UPLOAD_COMMAND_BUFFERS];
	Ring_Buffer ring_buffer;
} Asset_Upload_Command_Buffers;

#define MAX_FRAMES_IN_FLIGHT 2
GPU_Fence upload_fences[MAX_FRAMES_IN_FLIGHT];
GPU_Fence render_fences[MAX_FRAMES_IN_FLIGHT];
u32 current_frame_index;
Asset_Upload_Command_Buffers asset_upload_command_buffers;

#define RENDER_DEVICE_MEMORY_BLOCK_SIZE MEGABYTE(128)

void Create_Render_Display_Objects(Render_Context *render_context) {
	//GPU_Create_Swapchain(&render_context->gpu_context);
	//GPU_Compile_Render_Pass(&render_context->gpu_context, render_context->render_pass_count, render_context->render_passes);
}

void Initialize_Renderer(void *job_parameter_pointer) {
	Game_State *game_state = (Game_State *)job_parameter_pointer;
	//Initialize_Vulkan(game_state, &game_state->permanent_arena, &game_state->frame_arena);
	game_state->render_context.aspect_ratio = swapchain_image_width() / (f32)swapchain_image_height();
	game_state->render_context.focal_length = 0.1f;
	game_state->render_context.scene_projection = perspective_projection(game_state->camera.field_of_view, game_state->render_context.aspect_ratio, game_state->render_context.focal_length, 100.0f); // @TODO
	Create_Render_Memory_Block_Allocator(&game_state->render_context.memory.device_block_allocator, RENDER_DEVICE_MEMORY_BLOCK_SIZE, GPU_DEVICE_MEMORY);
	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		//GPU_Create_Fence(false, &upload_fences[i]);
		GPU_Create_Fence(true, &render_fences[i]);
	}

	// @TODO: Handpick these colors so they're visually distinct.
	for (u32 i = 0; i < RANDOM_COLOR_TABLE_LENGTH; i++) {
		random_color_table[i] = random_color();
	}
}

/*
// render pass:
// shadow {
//     material_properies contain cast_shadow

// material:
// rock {
//     cast_shadow,
//     receive_shadow,
//     physically_based,
//     albedo: aksdjf;laskjfd,
//     ...

typedef struct Render_Pass {
	GPU_Render_Pass gpu_render_pass;
	u32 eligible_submesh_count;
	u32 eligible_submeshes[100]; // @TODO @FRAME: Should really be per-frame.
	//which meshes are eligible for this render pass, change from frame to frame, calculated when the mesh is added to the scene
		//which meshes depends solely on the material for the mesh.
	//have to cross reference visible meshes and eligible meshes to find the list of meshes this render pass should operate on.
} Render_Pass;

u32 render_pass_count;
Render_Pass render_passes[100];

typedef struct Gather_Submeshes_For_Render_Pass_Job_Parameter {
	u32 visible_mesh_count;
	u32 *visible_meshes;
	u32 eligible_submesh_count;
	u32 *eligible_submeshes;
	//OUTPUT?
} Gather_Submeshes_For_Render_Pass_Job_Parameter;

void Gather_Submeshes_For_Render_Pass(void *job_parameter_pointer) {
	Gather_Submeshes_For_Render_Pass_Job_Parameter *job_parameter = job_parameter_pointer;
}
*/

// @TODO: GPU_Indexed_Render_Geometry
// @TODO: Read geometry directly into the staging buffer.
GPU_Mesh Upload_Indexed_Render_Geometry_To_GPU(u32 vertex_count, u32 size_of_vertex, void *vertices, u32 index_count, u32 *indices) {
	u32 vertices_size = vertex_count * size_of_vertex;
	u32 indices_size = + (index_count * sizeof(u32));
	void *staging_buffer = NULL;//GPU_Acquire_Host_Memory(vertices_size + indices_size);
	Copy_Memory(vertices, staging_buffer, vertices_size);
	Copy_Memory(indices, ((char *)staging_buffer) + vertices_size, indices_size);
	Render_Memory_Allocation *gpu_memory = NULL;//GPU_Acquire_Device_Memory(size);
	GPU_Command_List command_list;
	//GPU_Create_Command_List(vulkan_context.thread_local[thread_index].command_pools, &command_list);
	//GPU_Record_Copy_Buffer_Command(command_list, staging_buffer, gpu_memory, size);
	//GPU_End_Command_List(command_list);
	//Atomic_Write_To_Ring_Buffer(asset_upload_command_buffers, command_list);
	return (GPU_Mesh){
		.memory = gpu_memory,
		.indices_offset = vertices_size,
	};
}

// @TODO: Float up render passes, descriptor sets, pipelines, command lists.
void Render(Game_State *game_state) {
/*
	vulkan_context.currentFrame = vulkan_context.nextFrame;
	vulkan_context.nextFrame = (vulkan_context.nextFrame + 1) % VULKAN_MAX_FRAMES_IN_FLIGHT;
	vkWaitForFences(vulkan_context.device, 1, &vulkan_context.inFlightFences[vulkan_context.currentFrame], 1, UINT64_MAX);
	vkResetFences(vulkan_context.device, 1, &vulkan_context.inFlightFences[vulkan_context.currentFrame]);
	vkResetCommandPool(vulkan_context.device, context->thread_local[thread_index].command_pools[vulkan_context.currentFrame], 0);
	u32 swapchain_image_index = 0;
	VK_CHECK(vkAcquireNextImageKHR(vulkan_context.device, vulkan_context.swapchain, UINT64_MAX, vulkan_context.image_available_semaphores[vulkan_context.currentFrame], NULL, &swapchain_image_index));
	return swapchain_image_index;
*/

	// Flush uploads to GPU.
	{
		u32 count = asset_upload_command_buffers.ring_buffer.write_index - asset_upload_command_buffers.ring_buffer.read_index; // @TODO BROKEN
		if (count == 0) {
			return;
		}
		VkCommandBuffer *command_buffer;
		VkCommandBuffer buffers[100]; // @TODO
		VkSubmitInfo submit_info = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.commandBufferCount = count,
			.pCommandBuffers = buffers,
		};
		u32 i = 0;
		while (asset_upload_command_buffers.ring_buffer.read_index != asset_upload_command_buffers.ring_buffer.write_index) {
			// @TODO: Use a double buffer array?
			Atomic_Read_From_Ring_Buffer(asset_upload_command_buffers, command_buffer);
			buffers[i++] = *command_buffer;
		}
		VK_CHECK(vkQueueSubmit(vulkan_context.graphics_queue, 1, &submit_info, VK_NULL_HANDLE));
	}

	// Wait for command queues to empty and get next frame.
	{
		GPU_Fence fences[] = {
			upload_fences[vulkan_context.currentFrame],
			render_fences[vulkan_context.currentFrame],
		};
		GPU_Wait_For_Fences(ARRAY_COUNT(fences), fences, 1, UINT64_MAX);
		GPU_Reset_Fences(ARRAY_COUNT(fences), fences);
		VK_CHECK(vkAcquireNextImageKHR(vulkan_context.device, vulkan_context.swapchain, UINT64_MAX, vulkan_context.image_available_semaphores[vulkan_context.currentFrame], NULL, &vulkan_context.currentFrame));
		GPU_Reset_Command_List_Pool(game_state->gpu_context.thread_local[thread_index].command_pools[vulkan_context.currentFrame]);
	}

	//current_frame_index = GPU_Wait_For_Available_Frame(&game_state->gpu_context);

	//GPU_Fence asset_fence = GPU_Flush_Asset_Uploads();

	// Get a list of the meshes visible after culling.
	u32 visible_mesh_count = 0;
	u32 *visible_meshes = allocate_array(&game_state->frame_arena, u32, MAX_VISIBLE_ENTITY_MESHES);
	Frustum_Cull_Meshes(&game_state->camera, game_state->entities.meshes.bounding_spheres, game_state->entities.meshes.count, game_state->render_context.focal_length, game_state->render_context.aspect_ratio, visible_meshes, &visible_mesh_count);

/*
	for (u32 target_index = game_state->assets.waiting_for_gpu_upload.meshes.ring_buffer.write_index; game_state->assets.waiting_for_gpu_upload.meshes.ring_buffer.read_index != target_index;) {
		u32 read_index;
		VkFence *fence;
		Atomic_Read_From_Ring_Buffer_With_Index(game_state->assets.waiting_for_gpu_upload.meshes, fence, read_index);
		if (!fence) {
			continue;
		}
		if (!GPU_Was_Fence_Signalled(*fence)) {
			Atomic_Write_To_Ring_Buffer(game_state->assets.waiting_for_gpu_upload.meshes, *fence);
			continue;
		}
		game_state->assets.waiting_for_gpu_upload.meshes.assets[read_index]->load_status = ASSET_LOADED;
	}
	for (u32 target_index = game_state->assets.waiting_for_gpu_upload.materials.ring_buffer.write_index; game_state->assets.waiting_for_gpu_upload.materials.ring_buffer.read_index != target_index;) {
		u32 read_index;
		Material_GPU_Fences *fences;
		Atomic_Read_From_Ring_Buffer_With_Index(game_state->assets.waiting_for_gpu_upload.materials, fences, read_index);
		if (!fences) {
			continue;
		}
		for (s32 i = fences->count; i >= 0; i--) {
			if (!GPU_Was_Fence_Signalled(fences->fences[i])) {
				Atomic_Write_To_Ring_Buffer(game_state->assets.waiting_for_gpu_upload.materials, *fences);
				break;
			}
			fences->count--;
		}
		if (fences->count > 0) {
			continue;
		}
		game_state->assets.waiting_for_gpu_upload.materials.assets[read_index]->load_status = ASSET_LOADED;
	}
*/

/*
	// Check to see if the visible meshes are loaded into CPU and GPU memory.
	// If not, remove them from the visible mesh list. If it is, add it to the list of meshes to be drawn.
	//Scheduled_Mesh_Instance *scheduled_mesh_instances = malloc(sizeof(Scheduled_Mesh_Instance) * visible_mesh_count);
	for (u32 i = 0; i < visibile_mesh_count; i++) {
		u32 mesh_index = visible_meshes[i];
		Asset_Load_Status mesh_asset_load_status = game_state->meshes.instances[mesh_index].asset->load_status;
		if (mesh_asset_load_status == ASSET_LOADED) {
			continue;
		}
		if (mesh_asset_load_status == ASSET_LOADING_WAITING_FOR_GPU_UPLOAD) {
		}
		visible_mesh_count--;
		visible_meshes[i] = visible_meshes[visible_mesh_count];
		i--;
	}
	//u32 scheduled_mesh_instance_count = visible_mesh_count; // All meshes visible after

	for (u32 i = 0; i < render_pass_count; i++) {
		for (u32 j = 0; j < visible_mesh_count; j++) {
			for (u32 k = 0; k < render_passes[i].eligible_submesh_count; k++) {
				if (
			}
		}
	}
*/

/*
	// Update instance data.
	{
		// @TODO: Transfer all of this data at one time.
		// @TODO: Only update what needs to be updated.
		// ????? WRONG
		u32 offset_inside_instance_memory_segment = 0;
		for (u32 i = 0; i < visible_mesh_count; i++) {
			Instance_Data instance_data = {
				.material_id = visible_meshes[i],
			};
			GPU_Memory_Location destination = {
				NULL,
				vulkan_context.buffer_offsets.instance_memory_segment + offset_inside_instance_memory_segment,
			};
			GPU_Transfer_Data(&instance_data, destination, sizeof(instance_data));
			//stage_vulkan_data(&instance_data, sizeof(instance_data));
			//transfer_staged_vulkan_data(vulkan_context.buffer_offsets.instance_memory_segment + offset_inside_instance_memory_segment);
			offset_inside_instance_memory_segment += sizeof(Instance_Data);
		}
	}
*/

	//vulkan_submit(&game_state->camera, game_state->entities.meshes.instances, visible_meshes, visible_mesh_count, &render_context, frame_index); // @TODO
	game_state->render_context.debug_render_object_count = 0;
}

Texture_ID Upload_Texture_To_GPU(GPU_Context *context, u8 *pixels, s32 texture_width, s32 texture_height, GPU_Upload_Flags gpu_upload_flags) {
	return 0;
}

GPU_Mesh Upload_Render_Geometry_To_GPU(u32 vertex_count, u32 sizeof_vertex, void *vertices, u32 index_count, u32 *indices) {
	return (GPU_Mesh){};
}

/*
void Create_GPU_Buffer_Allocator(GPU_Buffer_Allocator *allocator, Render_Backend_Buffer_Usage_Flags buffer_usage_flags, Render_Backend_Memory_Type memory_type, u32 block_size) {
	allocator->buffer_usage_flags = buffer_usage_flags;
	allocator->block_buffer_count = 0;
	for (u32 i = 0; i < ARRAY_COUNT(allocator->block_buffers); i++) {
		Render_Backend_Create_Buffer(block_size, &allocator->block_buffers[i], buffer_usage_flags);
	}
	Render_Backend_Memory_Requirements memory_requirements;
	Render_Backend_Get_Buffer_Memory_Requirements(allocator->block_buffers[0], &memory_requirements);
	Create_GPU_Memory_Block_Allocator(&allocator->memory_block_allocator, block_size, memory_type, memory_requirements);
	//vkGetBufferMemoryRequirements(vulkan_context.device, new_block->buffer, &allocator->memory_requirements);
	//Render_Backend_Memory_Requirements memory_requirements;
	//Render_Backend_Get_Buffer_Memory_Requirements(&memory_requirements);
}
*/
