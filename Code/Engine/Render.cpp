#include "Render.h"
#include "render_generated.h"
#include "render_generated.c"
#include "Mesh.h"
#include "Camera.h"
#include "Jobs.h"

Render_Context renderContext;

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
			vertices[vertex_count++].position = (V3){x, y, z} + center;
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
	float r = rand() / (f32)RAND_MAX;
	float g = rand() / (f32)RAND_MAX;
	float b = rand() / (f32)RAND_MAX;
	return (V3){r, g, b};
}

#define MAX_VISIBLE_ENTITY_MESHES MAX_ENTITY_MESHES

// Render_Memory_Allocation ???
// GPU_Memory_Allocation ????

void Frustum_Cull_Meshes(Camera *camera, BoundingSphere *mesh_bounding_spheres, u32 mesh_bounding_sphere_count, f32 focal_length, f32 aspect_ratio, u32 *visible_meshes, u32 *visible_mesh_count) {
	f32 half_near_plane_height = focal_length * Tan(camera->fov / 2.0f);
	f32 half_near_plane_width = focal_length * Tan(camera->fov / 2.0f) * aspect_ratio;
	V3 center_of_near_plane = focal_length * camera->forward;
	enum {
		RIGHT,
		LEFT,
		TOP,
		BOTTOM,
		FRUSTUM_PLANE_COUNT,
	};
	V3 frustum_plane_normals[] = {
		[RIGHT] = CrossProduct(Normalize(center_of_near_plane + (half_near_plane_width * camera->side)), camera->up),
		[LEFT] = CrossProduct(camera->up, Normalize(center_of_near_plane - half_near_plane_width * camera->side)),
		[TOP] = CrossProduct(camera->side, Normalize(center_of_near_plane + half_near_plane_height * camera->up)),
		[BOTTOM] = CrossProduct(Normalize(center_of_near_plane - (half_near_plane_height * camera->up)), camera->side),
	};
	for (u32 i = 0; i < mesh_bounding_sphere_count; i++) {
		//Draw_Wire_Sphere(mesh_bounding_spheres[i].center, mesh_bounding_spheres[i].radius, v3_to_v4(random_color_table[i % RANDOM_COLOR_TABLE_LENGTH], 1.0f));
		if ((DotProduct(frustum_plane_normals[RIGHT], mesh_bounding_spheres[i].center) - DotProduct(frustum_plane_normals[RIGHT], camera->position) <= mesh_bounding_spheres[i].radius)
		 && (DotProduct(frustum_plane_normals[LEFT], mesh_bounding_spheres[i].center) - DotProduct(frustum_plane_normals[LEFT], camera->position) <= mesh_bounding_spheres[i].radius)
		 && (DotProduct(frustum_plane_normals[TOP], mesh_bounding_spheres[i].center) - DotProduct(frustum_plane_normals[TOP], camera->position) <= mesh_bounding_spheres[i].radius)
		 && (DotProduct(frustum_plane_normals[BOTTOM], mesh_bounding_spheres[i].center) - DotProduct(frustum_plane_normals[BOTTOM], camera->position) <= mesh_bounding_spheres[i].radius)) {
		 	Assert(*visible_mesh_count < MAX_VISIBLE_ENTITY_MESHES);
			visible_meshes[(*visible_mesh_count)++] = i;
		}
	}
}

enum {
	SHADOW_MAP_ATTACHMENT_ID,
	DEPTH_BUFFER_ATTACHMENT_ID,
	SWAPCHAIN_IMAGE_ATTACHMENT_ID,
};

GPU_Buffer uniform_buffer;
GPU_Fence matrix_fence;

void InitializeRenderer(void *job_parameter_pointer) {
	//GameState *game_state = (GameState *)job_parameter_pointer;
	//Render_Context *context = &game_state->render_context;
	auto context = &renderContext;
	Render_API_Initialize(&context->api_context);
	///Create_GPU_Memory_Block_Allocator(&context->gpu_memory_allocators.device_block_buffer, Megabyte(8), GPU_DEVICE_MEMORY);
	///Create_GPU_Memory_Block_Allocator(&context->gpu_memory_allocators.device_block_image, Megabyte(256), GPU_DEVICE_MEMORY);
	///Create_GPU_Memory_Block_Allocator(&context->gpu_memory_allocators.staging_block, Megabyte(256), GPU_HOST_MEMORY);
	// Descriptor sets.
	// Command Pools.
	// Create shaders.
	// Create display objects.
	// Initialize memory.
	//     Device block.
	//     Host ring buffer.
	//     Create uniform buffers.
	/*
	GPU_Image_Creation_Parameters shadow_map_image_creation_parameters = {
		.width = SHADOW_MAP_WIDTH,
		.height = SHADOW_MAP_HEIGHT,
		.format = GPU_FORMAT_D16_UNORM,
		.initial_layout = GPU_IMAGE_LAYOUT_UNDEFINED,
		.usage_flags = GPU_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT | GPU_IMAGE_USAGE_SAMPLED,
		.sample_count_flags = GPU_SAMPLE_COUNT_1,
	};
	*/
	GPU_Image shadow_map_image = Renderer::CreateGPUImage(SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, SHADOW_MAP_FORMAT, SHADOW_MAP_INITIAL_LAYOUT, SHADOW_MAP_IMAGE_USAGE_FLAGS, SHADOW_MAP_SAMPLE_COUNT_FLAGS);
	GPU_Image_View shadow_map_image_view = Render_API_Create_Image_View(&context->api_context, shadow_map_image, (VkFormat)SHADOW_MAP_FORMAT, SHADOW_MAP_IMAGE_USAGE_FLAGS);

	/*
	GPU_Image_Creation_Parameters depth_buffer_image_creation_parameters = {
		.width = window_width,
		.height = window_height,
		.format = DEPTH_BUFFER_FORMAT,
		.initial_layout = DEPTH_BUFFER_INITIAL_LAYOUT,
		.usage_flags = DEPTH_BUFFER_IMAGE_USAGE_FLAGS,
		.sample_count_flags = DEPTH_BUFFER_SAMPLE_COUNT_FLAGS,
	};
	*/
	GPU_Image depth_buffer_image = Renderer::CreateGPUImage(window_width, window_height, DEPTH_BUFFER_FORMAT, DEPTH_BUFFER_INITIAL_LAYOUT, DEPTH_BUFFER_IMAGE_USAGE_FLAGS, DEPTH_BUFFER_SAMPLE_COUNT_FLAGS);
	GPU_Image_View depth_buffer_image_view = Render_API_Create_Image_View(&context->api_context, depth_buffer_image, (VkFormat)DEPTH_BUFFER_FORMAT, DEPTH_BUFFER_IMAGE_USAGE_FLAGS);

	context->swapchain = Render_API_Create_Swapchain(&context->api_context);
	context->swapchain_image_count = Render_API_Get_Swapchain_Image_Count(&context->api_context, context->swapchain);
	GPU_Image_View *swapchain_image_views = (GPU_Image_View *)malloc(context->swapchain_image_count * sizeof(GPU_Image_View)); // @TODO
	Render_API_Get_Swapchain_Image_Views(&context->api_context, context->swapchain, context->swapchain_image_count, swapchain_image_views);

	for (s32 i = 0; i < GPU_MAX_FRAMES_IN_FLIGHT; i++) {
		context->inFlightFences[i] = RenderAPICreateFence(true);
	}

	Render_API_Load_Shaders(&context->api_context, context->shaders);

	context->descriptor_pool = Render_API_Initialize_Descriptors(&context->api_context, context->swapchain_image_count);

	context->render_passes.scene = TEMPORARY_Render_API_Create_Render_Pass(&context->api_context);

	context->framebuffers = (VkFramebuffer *)malloc(context->swapchain_image_count * sizeof(GPU_Framebuffer));
	for (s32 i = 0; i < context->swapchain_image_count; i++) {
		GPU_Image_View attachments[] = {
			swapchain_image_views[i],
			depth_buffer_image_view,
		};
		context->framebuffers[i] = Render_API_Create_Framebuffer(&context->api_context, context->render_passes.scene, window_width, window_height, ArrayCount(attachments), attachments);
	}

	context->render_thread_count = GetWorkerThreadCount();
	context->thread_local_contexts = (Thread_Local_Render_Context *)malloc(context->render_thread_count * sizeof(Thread_Local_Render_Context)); // @TODO
	for (s32 i = 0; i < context->render_thread_count; i++) {
		context->thread_local_contexts[i].upload_command_pool = RenderAPICreateCommandPool(GPU_GRAPHICS_COMMAND_QUEUE);
		context->thread_local_contexts[i].command_pools = (VkCommandPool *)malloc(context->swapchain_image_count * sizeof(GPU_Command_Pool));
		for (s32 j = 0; j < context->swapchain_image_count; j++) {
			context->thread_local_contexts[i].command_pools[j] = RenderAPICreateCommandPool(GPU_GRAPHICS_COMMAND_QUEUE);
		}
	}

	Create_Material_Pipelines(&context->api_context, context->shaders, context->render_passes.scene, context->pipelines);

	context->aspect_ratio = window_width / (f32)window_height;
	//context->focal_length = 0.1f;
	//context->scene_projection_matrix = perspective_projection(game_state->camera.field_of_view, context->aspect_ratio, context->focal_length, 100.0f); // @TODO

	// @TODO: Handpick these colors so they're visually distinct.
	for (u32 i = 0; i < RANDOM_COLOR_TABLE_LENGTH; i++) {
		random_color_table[i] = random_color();
	}

	context->rrdescriptor_sets = Create_Descriptor_Sets_For_Shader(context, context->swapchain_image_count, context->descriptor_pool, RUSTED_IRON_SHADER);

	#if 0
	VkWriteDescriptorSet descriptor_writes[10]; // @TODO
	s32 write_count = 0;
	uniform_buffer = Create_GPU_Device_Buffer(context, sizeof(M4) * context->swapchain_image_count + 0x100 * context->swapchain_image_count, GPU_UNIFORM_BUFFER | GPU_TRANSFER_DESTINATION_BUFFER);
	VkDescriptorBufferInfo buffer_infos[context->swapchain_image_count];
	s32 buffer_info_count = 0;
	for (s32 i = 0; i < context->swapchain_image_count; i++) {
		buffer_infos[buffer_info_count] = (VkDescriptorBufferInfo){
			.buffer = uniform_buffer,
			.offset = i * 0x100,
			.range  = sizeof(M4),
		};
		descriptor_writes[write_count++] = (VkWriteDescriptorSet){
			.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet          = context->descriptor_sets.rusted_iron_vertex_bind_per_object_update_immediate[i],
			.dstBinding      = 0,
			.dstArrayElement = 0,
			.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.pBufferInfo     = &buffer_infos[buffer_info_count],
		};
		buffer_info_count++;
	}
	vkUpdateDescriptorSets(context->api_context.device, write_count, descriptor_writes, 0, NULL);
	#endif

	matrix_fence = RenderAPICreateFence(false);

	for (s32 i = 0; i < context->swapchain_image_count; i++) {
		M4 m = IdentityMatrix();
		GPU_Descriptor_Update_Info update_infos[] = {
			{MODEL_TO_WORLD_SPACE_DESCRIPTOR, {.m4 = {}}},
		};
		Update_Descriptors(context, matrix_fence, i, &context->rrdescriptor_sets, ArrayCount(update_infos), update_infos);
		Render_API_Wait_For_Fences(1, &matrix_fence, true, U32_MAX);
		Render_API_Reset_Fences(1, &matrix_fence);
	}

	// @TODO
#if 0
	Render_Graph_External_Attachment external_attachments[] = {
		{
			.id = SHADOW_MAP_ATTACHMENT_ID,
			.image = shadow_map_image,
		},
		{
			.id = DEPTH_BUFFER_ATTACHMENT_ID,
			.image = depth_buffer_image,
		},
		{
			.id = SWAPCHAIN_IMAGE_ATTACHMENT_ID,
			.image = swapchain_images[0],
		},
	};
	Render_Pass_Attachment_Reference shadow_pass_depth_attachments = {
		.id = SHADOW_MAP_ATTACHMENT_ID,
		.usage = RENDER_ATTACHMENT_WRITE_ONLY,
	};
	Render_Pass_Attachment_Reference scene_pass_color_attachments[] = {
		{
			.id = SHADOW_MAP_ATTACHMENT_ID,
			.usage = RENDER_ATTACHMENT_READ_ONLY,
		},
		{
			.id = SWAPCHAIN_IMAGE_ATTACHMENT_ID,
			.usage = RENDER_ATTACHMENT_WRITE_ONLY,
		},
	};
	Render_Pass_Attachment_Reference scene_pass_depth_attachment = {
		.id = DEPTH_BUFFER_ATTACHMENT_ID,
		.usage = RENDER_ATTACHMENT_READ_WRITE,
	};
	Render_Pass_Description render_pass_descriptions[] = {
		{
			.depth_attachment = &shadow_pass_depth_attachments,
		},
		{
			.color_attachment_count = ArrayCount(scene_pass_color_attachments),
			.color_attachments = scene_pass_color_attachments,
			.depth_attachment = &scene_pass_depth_attachment,
		},
	};
	Render_Graph_Description render_graph_description = {
		.external_attachment_count = ArrayCount(external_attachments),
		.external_attachments = external_attachments,
		.render_pass_count = ArrayCount(render_pass_descriptions),
		.render_pass_descriptions = render_pass_descriptions,
	};
	GPU_Render_Graph render_graph = GPU_Compile_Render_Graph(&game_state->render_context.gpu_context, &render_graph_description);
#endif
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

void FinalizeAssetUploadsToGPU(Render_API_Context *api_context);

Array<MeshInstance> GetMeshInstances();

// @TODO: Float up render passes, descriptor sets, pipelines, command lists.
void Render() {
	auto camera = GetCamera("main");
	if (!camera) {
		LogPrint(LogType::ERROR, "Render: could not get main camera");
		return;
	}
	auto mesh_instances = GetMeshInstances();
	auto context = &renderContext;

	FinalizeAssetUploadsToGPU(&context->api_context);

	Render_API_Wait_For_Fences(1, &context->inFlightFences[context->currentFrame], true, U32_MAX);
	Render_API_Reset_Fences(1, &context->inFlightFences[context->currentFrame]);
	for (s32 i = 0; i < context->render_thread_count; i++) {
		Render_API_Reset_Command_Pool(&context->api_context, context->thread_local_contexts[i].command_pools[context->currentFrame]);
	}

	u32 swapchain_image_index = Render_API_Acquire_Next_Swapchain_Image_Index(&context->api_context, context->swapchain, context->currentFrame);

	#if 0
	GPU_Staging_Buffer staging_buffer;
	{
		M4 scene_projection_view_matrix = multiply_m4(camera->projectionMatrix, camera->viewMatrix);
		void *staging_memory;
		staging_buffer = Create_GPU_Staging_Buffer(context, sizeof(M4), &staging_memory);
		Copy_Memory(&scene_projection_view_matrix, staging_memory, sizeof(M4));
		GPU_Command_Buffer command_buffer = Render_API_Create_Command_Buffer(context->thread_local_contexts[thread_index].command_pools[context->currentFrame]);
		//Render_API_Record_Copy_Buffer_Command(&context->api_context, command_buffer, sizeof(M4), staging_buffer.buffer, uniform_buffer, staging_buffer.offset, sizeof(M4) * swapchain_image_index);
		Render_API_Record_Copy_Buffer_Command(&context->api_context, command_buffer, sizeof(M4), staging_buffer.buffer, uniform_buffer, staging_buffer.offset, 0x100 * swapchain_image_index);
		Render_API_End_Command_Buffer(command_buffer);
		Render_API_Submit_Command_Buffers(&context->api_context, 1, &command_buffer, GPU_GRAPHICS_COMMAND_QUEUE, matrix_fence);
	}
	#endif
	M4 m = camera->projectionMatrix * camera->viewMatrix;
	GPU_Descriptor_Update_Info update_infos[] = {
		{MODEL_TO_WORLD_SPACE_DESCRIPTOR, {.m4 = {}}},
		{COLOR_DESCRIPTOR, {.v4 = {}}},
	};
	CopyMemory(m.M, update_infos[0].data.m4, sizeof(m.M));
	V4 v = {1.0f, 0.0f, 0.0f, 1.0f};
	CopyMemory(&v, update_infos[1].data.v4, sizeof(v));
	Update_Descriptors(context, matrix_fence, swapchain_image_index, &context->rrdescriptor_sets, ArrayCount(update_infos), update_infos);

	//M4 scene_projection_view_matrix = multiply_m4(camera->projection_matrix, camera->view_matrix);
	//Descriptor_Set_Data descriptor_set_data[] = {
		//{
			//RUSTED_IRON_SHADER,
			//1,
			//{.rusted_iron = {m4_identity()}},
		//},
	//};
	//Update_Descriptor_Sets(context, ArrayCount(descriptor_set_data), descriptor_set_data, camera);

	GPU_Command_Buffer command_buffer = Render_API_Create_Command_Buffer(context->thread_local_contexts[threadIndex].command_pools[context->currentFrame]);
	Render_API_Record_Begin_Render_Pass_Command(&context->api_context, command_buffer, context->render_passes.scene, context->framebuffers[swapchain_image_index]);
	Render_API_Record_Set_Viewport_Command(&context->api_context, command_buffer, window_width, window_height);
	Render_API_Record_Set_Scissor_Command(&context->api_context, command_buffer, window_width, window_height);
	Render_API_Record_Bind_Pipeline_Command(&context->api_context, command_buffer, context->pipelines[RUSTED_IRON_SHADER]);
	for (auto &mesh : mesh_instances) {
		// @TODO
		if (mesh.asset->loadStatus != ASSET_LOADED) {
			continue;
		}
		//Render_API_Set_Per_Object_Shader_Parameters(&context->api_context);
		//Bind_Shader_Per_Object_Descriptor_Sets(context, RUSTED_IRON_SHADER);
		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context->pipelines[RUSTED_IRON_SHADER].layout, 0, 1, &context->rrdescriptor_sets.sets[swapchain_image_index], 0, NULL);
		Render_API_Record_Bind_Vertex_Buffer_Command(&context->api_context, command_buffer, mesh.asset->gpuMesh.vertex_buffer);
		Render_API_Record_Bind_Index_Buffer_Command(&context->api_context, command_buffer, mesh.asset->gpuMesh.index_buffer);
		u32 total_mesh_index_count = 0;
		for (auto indexCount : mesh.asset->submeshIndexCounts) {
			//Render_API_Set_Per_Material_Shader_Parameters(&context->api_context, RUSTED_IRON_SHADER); // @TODO: WRONG!
			Render_API_Draw_Indexed_Vertices(&context->api_context, command_buffer, indexCount, total_mesh_index_count);
			total_mesh_index_count += indexCount;
		}
	}
	Render_API_Record_End_Render_Pass_Command(&context->api_context, command_buffer);
	Render_API_End_Command_Buffer(command_buffer);
	Render_API_Wait_For_Fences(1, &matrix_fence, true, U32_MAX);
	TEMPORARY_VULKAN_SUBMIT(&context->api_context, command_buffer, context->currentFrame, context->inFlightFences[context->currentFrame]);

	Render_API_Present_Swapchain_Image(&context->api_context, context->swapchain, swapchain_image_index, context->currentFrame);

	// @TODO: WRONG Asset system needs its own block allocator so it doesn't get messed up.
	Render_API_Reset_Fences(1, &matrix_fence);
	//Render_API_Destroy_Buffer(&context->api_context, staging_buffer.buffer);
	// @TODO Renderer::ClearGPUMemoryBlockAllocator(&gpuContext.memoryAllocatory.stagingBlock); // @TODO
	Renderer::ClearGPUMemoryBlockAllocator(); // @TODO

	context->currentFrame = (context->currentFrame + 1) % GPU_MAX_FRAMES_IN_FLIGHT;
#if 0
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
		GPU_Wait_For_Fences(ArrayCount(fences), fences, 1, UINT64_MAX);
		GPU_Reset_Fences(ArrayCount(fences), fences);
		VK_CHECK(vkAcquireNextImageKHR(vulkan_context.device, vulkan_context.swapchain, UINT64_MAX, vulkan_context.image_available_semaphores[vulkan_context.currentFrame], NULL, &vulkan_context.currentFrame));
		GPU_Reset_Command_List_Pool(game_state->render_context.thread_local[thread_index].command_list_pools[game_state->render_context.current_frame_index]);
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
#endif
	//vulkan_submit(&game_state->camera, game_state->entities.meshes.instances, visible_meshes, visible_mesh_count, &render_context, frame_index); // @TODO
	context->debug_render_object_count = 0;
}

/*
void Create_GPU_Buffer_Allocator(GPU_Buffer_Allocator *allocator, Render_Backend_Buffer_Usage_Flags buffer_usage_flags, Render_Backend_Memory_Type memory_type, u32 block_size) {
	allocator->buffer_usage_flags = buffer_usage_flags;
	allocator->block_buffer_count = 0;
	for (u32 i = 0; i < ArrayCount(allocator->block_buffers); i++) {
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
