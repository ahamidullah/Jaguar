s64 RenderWidth()
{
	return 1200;
}

s64 RenderHeight()
{
	return 1000;
}

void InitializeRenderer(void *)
{
}

void Render()
{
}

#if 0
#include "Render.h"
#include "ShaderGlobal.h"
#include "Mesh.h"
#include "Camera.h"
#include "Shader.h"

#include "Basic/HashTable.h"
#include "Basic/File.h"
#include "Basic/Process.h"

#define VERTEX_BUFFER_BIND_ID 0
#define INITIAL_DESCRIPTOR_SET_BUFFER_SIZE MegabytesToBytes(1)

struct RenderContext
{
	f32 aspectRatio;

	s64 frameIndex;
	GfxFence frameFences[GFX_MAX_FRAMES_IN_FLIGHT];

	s64 swapchainImageIndex; // @TODO
	//s64 swapchainImageCount;
	GfxSwapchain swapchain;
	Array<GfxImage> swapchainImages;
	Array<GfxImageView> swapchainImageViews;

	GfxPipelineLayout pipelineLayout;

	GfxDescriptorPool descriptorPool;
	Array<GfxDescriptorSetLayout> descriptorSetLayouts;
	Array<Array<GfxDescriptorSet>> descriptorSets;
	Array<Array<GfxBuffer>> descriptorSetBuffers;
	GfxFence descriptorSetUpdateFence; // @TODO: Delete me.

	// @TODO TEMPORARY
	GfxFramebuffer *framebuffers;

	GfxSemaphore swapchainImageAcquiredSemaphores[GFX_MAX_FRAMES_IN_FLIGHT];
	//GfxSemaphore drawCompleteSemaphores[GFX_MAX_FRAMES_IN_FLIGHT];
} renderGlobals;

//s64 GetFrameIndex()
//{
	//return renderGlobals.frameIndex;
//}

void CreateDescriptorSetGroup(s64 setIndex, s64 bindingInfoCount, DescriptorSetBindingInfo *bindingInfos)
{
	renderGlobals.descriptorSetLayouts[setIndex] = GfxCreateDescriptorSetLayout(bindingInfoCount, bindingInfos);
	for (auto i = 0; i < renderGlobals.swapchainImages.count; i++)
	{
		GfxCreateDescriptorSets(renderGlobals.descriptorPool, renderGlobals.descriptorSetLayouts[setIndex], 1, &renderGlobals.descriptorSets[i][setIndex]);
		renderGlobals.descriptorSetBuffers[i][setIndex] = CreateGPUBuffer(INITIAL_DESCRIPTOR_SET_BUFFER_SIZE, GFX_UNIFORM_BUFFER | GFX_TRANSFER_DESTINATION_BUFFER, GFX_GPU_ONLY_MEMORY, GPU_RESOURCE_LIFETIME_PERSISTENT);
	}
}

GfxPipeline GetShaderGraphicsPipeline(Shader *shader, GfxRenderPass renderPass)
{
	if (shader->name == "Model")
	{
		GfxFramebufferAttachmentColorBlendDescription colorBlendDescription =
		{
			.enable_blend = true,
			.source_color_blend_factor = GFX_BLEND_FACTOR_SRC_ALPHA,
			.destination_color_blend_factor = GFX_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			//.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
			//.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
			.color_blend_operation = GFX_BLEND_OP_ADD ,
			.source_alpha_blend_factor = GFX_BLEND_FACTOR_ONE,
			.destination_alpha_blend_factor = GFX_BLEND_FACTOR_ZERO,
			.alpha_blend_operation = GFX_BLEND_OP_ADD,
			.color_write_mask = GFX_COLOR_COMPONENT_RED | GFX_COLOR_COMPONENT_GREEN | GFX_COLOR_COMPONENT_BLUE | GFX_COLOR_COMPONENT_ALPHA,
		};
		GfxPipelineVertexInputBindingDescription vertexInputBindingDescriptions[] =
		{
			{
				.binding = VERTEX_BUFFER_BIND_ID,
				.stride = sizeof(Vertex1P1N),
				.input_rate = GFX_VERTEX_INPUT_RATE_VERTEX,
			},
		};
		GfxPipelineVertexInputAttributeDescription vertexInputAttributeDescriptions[] =
		{
			{
				.format = GFX_FORMAT_R32G32B32_SFLOAT,
				.binding = VERTEX_BUFFER_BIND_ID,
				.location = 0,
				.offset = offsetof(Vertex1P1N, position),
			},
			{
				.format = GFX_FORMAT_R32G32B32_SFLOAT,
				.binding = VERTEX_BUFFER_BIND_ID,
				.location = 1,
				.offset = offsetof(Vertex1P1N, normal),
			},
		};
		GfxDynamicPipelineState dynamicStates[] =
		{
			GFX_DYNAMIC_PIPELINE_STATE_VIEWPORT,
			GFX_DYNAMIC_PIPELINE_STATE_SCISSOR,
		};
		GfxPipelineDescription pipelineDescription =
		{
			.layout = renderGlobals.pipelineLayout,
			.topology = GFX_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.viewport_width = (f32)GetRenderWidth(),
			.viewport_height = (f32)GetRenderHeight(),
			.scissor_width = (u32)GetRenderWidth(),
			.scissor_height = (u32)GetRenderHeight(),
			.depth_compare_operation = GFX_COMPARE_OP_LESS,
			.framebuffer_attachment_color_blend_count = 1,
			.framebuffer_attachment_color_blend_descriptions = &colorBlendDescription,
			.vertex_input_attribute_count = CArrayCount(vertexInputAttributeDescriptions),
			.vertex_input_attribute_descriptions = vertexInputAttributeDescriptions,
			.vertex_input_binding_count = CArrayCount(vertexInputBindingDescriptions),
			.vertex_input_binding_descriptions = vertexInputBindingDescriptions,
			.dynamic_state_count = CArrayCount(dynamicStates),
			.dynamic_states = dynamicStates,
			.shaderStages = shader->stages,
			.shaderModules = shader->modules,
			.render_pass = renderPass,
			.enable_depth_bias = false,
		};
		return GfxCreatePipeline(pipelineDescription);
	}
	Abort("Can't handle shader %k.\n", shader->name);
	return GfxPipeline{};
}

struct RenderGraphResource
{
	String name;
	bool isTexture = false;
	bool isBuffer = false;
};

struct RenderPass
{
	String name;
	Shader *shader;
	Array<RenderGraphResource> read;
	Array<RenderGraphResource> write;
	Array<RenderGraphResource> create;
	void (*execute)(GPUCommandBuffer commandBuffer);
};

struct PhyisicalRenderPass
{
	GfxRenderPass gfxPass;
	GfxPipeline gfxPipeline;
	void (*execute)(GPUCommandBuffer commandBuffer);
};

struct RenderGraph
{
	bool compiled = false;
	Array<RenderPass> logicalPasses;
	Array<PhyisicalRenderPass> physicalPasses;
};

void AddRenderGraphPass(RenderGraph *graph, const RenderPass &pass)
{
	ArrayAppend(&graph->logicalPasses, pass);
}

void CompileRenderGraph(RenderGraph *graph)
{
	graph->compiled = true;
	for (const auto &logicalPass : graph->logicalPasses)
	{
		PhyisicalRenderPass physicalPass =
		{
			.gfxPass = TEMPORARY_Render_API_Create_Render_Pass(),
			.gfxPipeline = GetShaderGraphicsPipeline(logicalPass.shader, physicalPass.gfxPass),
			.execute = logicalPass.execute,
		};
		ArrayAppend(&graph->physicalPasses, physicalPass);
	}
}

GfxBuffer uniform_buffer;
GfxImageView depthBufferImageView; // @TODO

void ExecuteRenderGraph(RenderGraph *graph, s64 swapchainImageIndex)
{
	for (const auto &pass : graph->physicalPasses)
	{
		GfxImageView attachments[] =
		{
			renderGlobals.swapchainImageViews[swapchainImageIndex],
			depthBufferImageView,
		};
		renderGlobals.framebuffers[swapchainImageIndex] = GfxCreateFramebuffer(pass.gfxPass, GetRenderWidth(), GetRenderHeight(), CArrayCount(attachments), attachments);

		auto commandBuffer = CreateGPUCommandBuffer(GFX_GRAPHICS_COMMAND_QUEUE, GPU_RESOURCE_LIFETIME_FRAME);

		GfxRecordBindDescriptorSetsCommand(commandBuffer, GFX_GRAPHICS_PIPELINE_BIND_POINT, renderGlobals.pipelineLayout, 0, ArrayLength(renderGlobals.descriptorSets[swapchainImageIndex]), &renderGlobals.descriptorSets[swapchainImageIndex][0]);

		GfxRecordBeginRenderPassCommand(commandBuffer, pass.gfxPass, renderGlobals.framebuffers[swapchainImageIndex]);
		GfxRecordBindPipelineCommand(commandBuffer, pass.gfxPipeline);
		pass.execute(commandBuffer);

		QueueGPUCommandBuffer(commandBuffer, NULL);

		//GfxWaitForFences(1, &renderGlobals.descriptorSetUpdateFence, true, U32_MAX);

		//GfxSubmitInfo submitInfo;
		//ArrayAppend(&submitInfo.commandBuffers, commandBuffer);
		//GfxSubmitCommandBuffers(GFX_GRAPHICS_COMMAND_QUEUE, submitInfo, renderGlobals.frameFences[renderGlobals.frameIndex]);

		//GfxResetFences(1, &renderGlobals.descriptorSetUpdateFence);
	}
}

void InitializeRenderer(void *jobParam)
{
	InitializeGPU((OSWindow *)jobParam);

	//auto shadowMapImage = CreateGPUImage(SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, SHADOW_MAP_FORMAT, SHADOW_MAP_INITIAL_LAYOUT, SHADOW_MAP_IMAGE_USAGE_FLAGS, SHADOW_MAP_SAMPLE_COUNT_FLAGS);
	//auto shadowMapImageView = GfxCreateImageView(shadowMapImage, SHADOW_MAP_FORMAT, SHADOW_MAP_IMAGE_USAGE_FLAGS);

	{
		auto depthImg = NewGPUImage();
		//const auto DEPTH_BUFFER_INITIAL_LAYOUT = GFX_IMAGE_LAYOUT_UNDEFINED;
		//const auto DEPTH_BUFFER_FORMAT = GFX_FORMAT_D32_SFLOAT_S8_UINT;
		//const auto DEPTH_BUFFER_IMAGE_USAGE_FLAGS = GFX_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT;
		//const auto DEPTH_BUFFER_SAMPLE_COUNT_FLAGS = GFX_SAMPLE_COUNT_1;

		//auto depthBufferImage = CreateGPUImage(GetRenderWidth(), GetRenderHeight(), DEPTH_BUFFER_FORMAT, DEPTH_BUFFER_INITIAL_LAYOUT, DEPTH_BUFFER_IMAGE_USAGE_FLAGS, DEPTH_BUFFER_SAMPLE_COUNT_FLAGS, GFX_GPU_ONLY_MEMORY, GPU_RESOURCE_LIFETIME_PERSISTENT);

		auto swizzleMapping = GfxSwizzleMapping
		{
			.r = GFX_SWIZZLE_MAPPING_IDENTITY,
			.g = GFX_SWIZZLE_MAPPING_IDENTITY,
			.b = GFX_SWIZZLE_MAPPING_IDENTITY,
			.a = GFX_SWIZZLE_MAPPING_IDENTITY,
		};
		auto subresourceRange = GfxImageSubresourceRange
		{
			.aspectMask = GFX_IMAGE_ASPECT_DEPTH,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		};
		depthBufferImageView = GfxCreateImageView(depthBufferImage, GFX_IMAGE_VIEW_TYPE_2D, DEPTH_BUFFER_FORMAT, swizzleMapping, subresourceRange);
	}

	for (auto i = 0; i < GFX_MAX_FRAMES_IN_FLIGHT; i++)
	{
		renderGlobals.frameFences[i] = GfxCreateFence(true);
	}

	LoadShaders();

	renderGlobals.descriptorPool = GfxCreateDescriptorPool();

	//ResizeArray(&renderGlobals.threadLocal, GetWorkerThreadCount());

	renderGlobals.framebuffers = AllocateArrayMemory(GfxFramebuffer, renderGlobals.swapchainImages.count);

	ResizeArray(&renderGlobals.descriptorSets, renderGlobals.swapchainImages.count);
	ResizeArray(&renderGlobals.descriptorSetBuffers, renderGlobals.swapchainImages.count);
	ResizeArray(&renderGlobals.descriptorSetLayouts, DESCRIPTOR_SET_COUNT);
	for (auto i = 0; i < renderGlobals.swapchainImages.count; i++)
	{
		ResizeArray(&renderGlobals.descriptorSets[i], DESCRIPTOR_SET_COUNT);
		ResizeArray(&renderGlobals.descriptorSetBuffers[i], DESCRIPTOR_SET_COUNT);
	}
	renderGlobals.descriptorSetUpdateFence = GfxCreateFence(false);
	{
		DescriptorSetBindingInfo globalBindingInfos[] =
		{
			{
				.binding = 0,
				.descriptorType = GFX_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.descriptorCount = 1,
				.stageFlags = GFX_VERTEX_SHADER_STAGE,
			},
		};
		CreateDescriptorSetGroup(GLOBAL_DESCRIPTOR_SET_INDEX, CArrayCount(globalBindingInfos), globalBindingInfos);

		DescriptorSetBindingInfo viewBindingInfos[] =
		{
			{
				.binding = 0,
				.descriptorType = GFX_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.descriptorCount = 1,
				.stageFlags = GFX_VERTEX_SHADER_STAGE,
			},
		};
		CreateDescriptorSetGroup(VIEW_DESCRIPTOR_SET_INDEX, CArrayCount(viewBindingInfos), viewBindingInfos);

		DescriptorSetBindingInfo materialBindingInfos[] =
		{
			{
				.binding = 0,
				.descriptorType = GFX_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.descriptorCount = 1,
				.stageFlags = GFX_VERTEX_SHADER_STAGE | GFX_FRAGMENT_SHADER_STAGE,
			},
		};
		CreateDescriptorSetGroup(MATERIAL_DESCRIPTOR_SET_INDEX, CArrayCount(materialBindingInfos), materialBindingInfos);

		DescriptorSetBindingInfo objectBindingInfos[] =
		{
			{
				.binding = 0,
				.descriptorType = GFX_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.descriptorCount = 1,
				.stageFlags = GFX_VERTEX_SHADER_STAGE,
			},
		};
		CreateDescriptorSetGroup(OBJECT_DESCRIPTOR_SET_INDEX, CArrayCount(objectBindingInfos), objectBindingInfos);
	}

	renderGlobals.pipelineLayout = GfxCreatePipelineLayout(ArrayLength(renderGlobals.descriptorSetLayouts), &renderGlobals.descriptorSetLayouts[0]);

	renderGlobals.aspectRatio = GetRenderWidth() / (f32)GetRenderHeight();

	for (auto i = 0; i < GFX_MAX_FRAMES_IN_FLIGHT; i++)
	{
		renderGlobals.swapchainImageAcquiredSemaphores[i] = GfxCreateSemaphore();
	}

#if 0
	for (auto i = 0; i < renderGlobals.swapchainImageCount; i++)
	{
		M4 m = IdentityMatrix();
		//GPU_Descriptor_Update_Info update_infos[] = {
			//{MODEL_TO_WORLD_SPACE_DESCRIPTOR, {.m4 = {}}},
		//};
		//Update_Descriptors(context, matrix_fence, i, &renderGlobals.rrdescriptor_sets, CArrayCount(update_infos), update_infos);
		void *staging_memory;
		GfxBuffer staging_buffer = CreateGPUBuffer(sizeof(M4), GFX_TRANSFER_SOURCE_BUFFER, GFX_CPU_TO_GPU_MEMORY, GPU_RESOURCE_LIFETIME_FRAME, &staging_memory);
		CopyMemory(&m, staging_memory, sizeof(M4));
		auto commandBuffer = CreateGPUCommandBuffer(GFX_GRAPHICS_COMMAND_QUEUE, GPU_RESOURCE_LIFETIME_FRAME);
		GfxRecordCopyBufferCommand(commandBuffer, sizeof(M4), staging_buffer, renderGlobals.descriptorSetBuffers[i][OBJECT_DESCRIPTOR_SET_INDEX], 0, 0);
		GfxSubmitCommandBuffers(1, &commandBuffer, GFX_GRAPHICS_COMMAND_QUEUE, matrix_fence);
		GfxUpdateDescriptorSets(renderGlobals.descriptorSets[i][OBJECT_DESCRIPTOR_SET_INDEX], renderGlobals.descriptorSetBuffers[i][OBJECT_DESCRIPTOR_SET_INDEX], GFX_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 0, sizeof(M4));

		GfxWaitForFences(1, &matrix_fence, true, U32_MAX);
		GfxResetFences(1, &matrix_fence);
	}

	// @TODO
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
			.color_attachment_count = CArrayCount(scene_pass_color_attachments),
			.color_attachments = scene_pass_color_attachments,
			.depth_attachment = &scene_pass_depth_attachment,
		},
	};
	Render_Graph_Description render_graph_description = {
		.external_attachment_count = CArrayCount(external_attachments),
		.external_attachments = external_attachments,
		.render_pass_count = CArrayCount(render_pass_descriptions),
		.render_pass_descriptions = render_pass_descriptions,
	};
	GPU_Render_Graph render_graph = GPU_Compile_Render_Graph(&game_state->render_context.gpu_context, &render_graph_description);
#endif
}

Array<MeshInstance> GetMeshInstances();

void Render()
{
	StartGPUFrame();
	auto camera = GetCamera("main");
	if (!camera)
	{
		LogError("Render: could not get main camera");
		return;
	}
	GPURenderFrameFinish();

	GfxWaitForFences(1, &renderGlobals.frameFences[renderGlobals.frameIndex], true, U32_MAX);
	GfxResetFences(1, &renderGlobals.frameFences[renderGlobals.frameIndex]);

	auto swapchainImageIndex = GfxAcquireNextSwapchainImage(renderGlobals.swapchain, renderGlobals.swapchainImageAcquiredSemaphores[renderGlobals.frameIndex]);

	ClearGPUMemoryForFrameIndex(renderGlobals.frameIndex);
	ClearGPUCommandPoolsForFrameIndex(renderGlobals.frameIndex);

	// Update descriptor sets.
	{
		auto commandBuffer = CreateGPUCommandBuffer(GFX_TRANSFER_COMMAND_QUEUE, GPU_RESOURCE_LIFETIME_FRAME);
		u32 u = 1;
		{
			void *stagingMemory;
			auto stagingBuffer = CreateGPUBuffer(sizeof(u32), GFX_TRANSFER_SOURCE_BUFFER, GFX_CPU_TO_GPU_MEMORY, GPU_RESOURCE_LIFETIME_FRAME, &stagingMemory);
			CopyMemory(&u, stagingMemory, sizeof(u32));
			GfxRecordCopyBufferCommand(commandBuffer, sizeof(u32), stagingBuffer, renderGlobals.descriptorSetBuffers[swapchainImageIndex][GLOBAL_DESCRIPTOR_SET_INDEX], 0, 0);
			GfxUpdateDescriptorSets(renderGlobals.descriptorSets[swapchainImageIndex][GLOBAL_DESCRIPTOR_SET_INDEX], renderGlobals.descriptorSetBuffers[swapchainImageIndex][GLOBAL_DESCRIPTOR_SET_INDEX], GFX_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 0, sizeof(u32));
		}
		{
			void *stagingMemory;
			auto stagingBuffer = CreateGPUBuffer(sizeof(u32), GFX_TRANSFER_SOURCE_BUFFER, GFX_CPU_TO_GPU_MEMORY, GPU_RESOURCE_LIFETIME_FRAME, &stagingMemory);
			CopyMemory(&u, stagingMemory, sizeof(u32));
			GfxRecordCopyBufferCommand(commandBuffer, sizeof(u32), stagingBuffer, renderGlobals.descriptorSetBuffers[swapchainImageIndex][VIEW_DESCRIPTOR_SET_INDEX], 0, 0);
			GfxUpdateDescriptorSets(renderGlobals.descriptorSets[swapchainImageIndex][VIEW_DESCRIPTOR_SET_INDEX], renderGlobals.descriptorSetBuffers[swapchainImageIndex][VIEW_DESCRIPTOR_SET_INDEX], GFX_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 0, sizeof(u32));
		}
		{
			void *stagingMemory;
			auto stagingBuffer = CreateGPUBuffer(sizeof(u32), GFX_TRANSFER_SOURCE_BUFFER, GFX_CPU_TO_GPU_MEMORY, GPU_RESOURCE_LIFETIME_FRAME, &stagingMemory);
			CopyMemory(&u, stagingMemory, sizeof(u32));
			GfxRecordCopyBufferCommand(commandBuffer, sizeof(u32), stagingBuffer, renderGlobals.descriptorSetBuffers[swapchainImageIndex][MATERIAL_DESCRIPTOR_SET_INDEX], 0, 0);
			GfxUpdateDescriptorSets(renderGlobals.descriptorSets[swapchainImageIndex][MATERIAL_DESCRIPTOR_SET_INDEX], renderGlobals.descriptorSetBuffers[swapchainImageIndex][MATERIAL_DESCRIPTOR_SET_INDEX], GFX_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 0, sizeof(u32));
		}
		{
			auto projectionMatrix = CreateInfinitePerspectiveProjectionMatrix(0.01f, camera->fov, renderGlobals.aspectRatio);
			auto viewMatrix = CreateViewMatrix(camera->transform.position, CalculateForwardVector(camera->transform.rotation));
			//auto viewMatrix = CreateViewMatrix({20000, -20000, 20000}, Normalize(V3{0, 0, 0} - V3{20000, -20000, 20000}));
			//SetRotationMatrix(&viewMatrix, ToMatrix(camera->transform.rotation));
			//auto viewMatrix = ViewMatrix(camera->transform.position, -camera->transform.position);
			//M4 m = projectionMatrix * viewMatrix;
			M4 m = projectionMatrix * viewMatrix;
			void *stagingMemory;
			auto stagingBuffer = CreateGPUBuffer(sizeof(M4), GFX_TRANSFER_SOURCE_BUFFER, GFX_CPU_TO_GPU_MEMORY, GPU_RESOURCE_LIFETIME_FRAME, &stagingMemory);
			CopyMemory(&m, stagingMemory, sizeof(M4));
			GfxRecordCopyBufferCommand(commandBuffer, sizeof(M4), stagingBuffer, renderGlobals.descriptorSetBuffers[swapchainImageIndex][OBJECT_DESCRIPTOR_SET_INDEX], 0, 0);
			GfxUpdateDescriptorSets(renderGlobals.descriptorSets[swapchainImageIndex][OBJECT_DESCRIPTOR_SET_INDEX], renderGlobals.descriptorSetBuffers[swapchainImageIndex][OBJECT_DESCRIPTOR_SET_INDEX], GFX_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 0, sizeof(M4));
		}

		QueueGPUCommandBuffer(commandBuffer, NULL);

		//GfxSubmitInfo submitInfo;
		//ArrayAppend(&submitInfo.commandBuffers, commandBuffer);
		//GfxSubmitCommandBuffers(GFX_GRAPHICS_COMMAND_QUEUE, submitInfo, renderGlobals.descriptorSetUpdateFence);
	}

	auto frameTransfersCompleteSemaphore = SubmitQueuedGPUCommandBuffers(GFX_TRANSFER_COMMAND_QUEUE, {}, {}, NULL);

	// Build and execute render graph.
	{
		RenderGraph renderGraph;
		RenderPass renderPass =
		{
			.name = "test",
			.shader = GetShader("Model"),
			.execute = [](GPUCommandBuffer commandBuffer)
			{
				GfxRecordSetViewportCommand(commandBuffer, GetRenderWidth(), GetRenderHeight());
				GfxRecordSetScissorCommand(commandBuffer, GetRenderWidth(), GetRenderHeight());
				for (auto &model : GetModelInstances())
				{
					auto modelAsset = LockAsset(model.assetName);
					if (!modelAsset)
					{
						continue;
					}
					Defer(UnlockAsset(model.assetName));
					for (auto &submesh : modelAsset->mesh.submeshes)
					{
						GfxRecordBindVertexBufferCommand(commandBuffer, modelAsset->mesh.gpuGeometry.vertexBuffer);
						GfxRecordBindIndexBufferCommand(commandBuffer, modelAsset->mesh.gpuGeometry.indexBuffer);
						GfxDrawIndexedVertices(commandBuffer, submesh.indexCount, submesh.firstIndex, submesh.vertexOffset);
					}
				}
				GfxRecordEndRenderPassCommand(commandBuffer);
			},
		};
		AddRenderGraphPass(&renderGraph, renderPass);
		CompileRenderGraph(&renderGraph);
		ExecuteRenderGraph(&renderGraph, swapchainImageIndex);
	}

	auto ss = Array<GfxSemaphore>{};
	ArrayAppend(&ss, frameTransfersCompleteSemaphore);
	ArrayAppend(&ss, renderGlobals.swapchainImageAcquiredSemaphores[renderGlobals.frameIndex]);

	auto ww = Array<GfxPipelineStageFlags>{};
	ArrayAppend(&ww, GFX_PIPELINE_STAGE_TOP_OF_PIPE);
	ArrayAppend(&ww, GFX_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT);

	auto frameGraphicsCompleteSemaphore = SubmitQueuedGPUCommandBuffers(GFX_GRAPHICS_COMMAND_QUEUE, ss, ww, renderGlobals.frameFences[renderGlobals.frameIndex]);

	GfxPresentSwapchainImage(renderGlobals.swapchain, swapchainImageIndex, CreateArray<GfxSemaphore>(1, &frameGraphicsCompleteSemaphore));

	renderGlobals.frameIndex = (renderGlobals.frameIndex + 1) % GFX_MAX_FRAMES_IN_FLIGHT;

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
			upload_fences[vulkan_context.frameIndex],
			render_fences[vulkan_context.frameIndex],
		};
		GPU_Wait_For_Fences(CArrayCount(fences), fences, 1, UINT64_MAX);
		GPU_Reset_Fences(CArrayCount(fences), fences);
		VK_CHECK(vkAcquireNextImageKHR(vulkan_context.device, vulkan_context.swapchain, UINT64_MAX, vulkan_context.image_available_semaphores[vulkan_context.frameIndex], NULL, &vulkan_context.frameIndex));
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
	//renderGlobals.debugRenderObjectCount = 0;
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

/*
#define MAX_VISIBLE_ENTITY_MESHES MAX_ENTITY_MESHES

void Frustum_Cull_Meshes(Camera *camera, BoundingSphere *mesh_bounding_spheres, u32 mesh_bounding_sphere_count, f32 focal_length, f32 aspect_ratio, u32 *visible_meshes, u32 *visible_mesh_count) {
	f32 half_near_plane_height = focal_length * Tan(camera->fov / 2.0f);
	f32 half_near_plane_width = focal_length * Tan(camera->fov / 2.0f) * aspect_ratio;
	V3 center_of_near_plane = focal_length * CalculateForwardVector(camera->transform.rotation);
	enum {
		RIGHT,
		LEFT,
		TOP,
		BOTTOM,
		FRUSTUM_PLANE_COUNT,
	};
	auto right = CalculateTransformRight(&camera->transform);
	auto up = CalculateTransformUp(&camera->transform);
	V3 frustum_plane_normals[] = {
		[RIGHT] = CrossProduct(Normalize(center_of_near_plane + (half_near_plane_width * right)), up),
		[LEFT] = CrossProduct(up, Normalize(center_of_near_plane - half_near_plane_width * right)),
		[TOP] = CrossProduct(right, Normalize(center_of_near_plane + half_near_plane_height * up)),
		[BOTTOM] = CrossProduct(Normalize(center_of_near_plane - (half_near_plane_height * up)), right),
	};
	for (u32 i = 0; i < mesh_bounding_sphere_count; i++) {
		//Draw_Wire_Sphere(mesh_bounding_spheres[i].center, mesh_bounding_spheres[i].radius, v3_to_v4(random_color_table[i % RANDOM_COLOR_TABLE_LENGTH], 1.0f));
		if ((DotProduct(frustum_plane_normals[RIGHT], mesh_bounding_spheres[i].center) - DotProduct(frustum_plane_normals[RIGHT], camera->transform.position) <= mesh_bounding_spheres[i].radius)
		 && (DotProduct(frustum_plane_normals[LEFT], mesh_bounding_spheres[i].center) - DotProduct(frustum_plane_normals[LEFT], camera->transform.position) <= mesh_bounding_spheres[i].radius)
		 && (DotProduct(frustum_plane_normals[TOP], mesh_bounding_spheres[i].center) - DotProduct(frustum_plane_normals[TOP], camera->transform.position) <= mesh_bounding_spheres[i].radius)
		 && (DotProduct(frustum_plane_normals[BOTTOM], mesh_bounding_spheres[i].center) - DotProduct(frustum_plane_normals[BOTTOM], camera->transform.position) <= mesh_bounding_spheres[i].radius)) {
		 	Assert(*visible_mesh_count < MAX_VISIBLE_ENTITY_MESHES);
			visible_meshes[(*visible_mesh_count)++] = i;
		}
	}
}
*/
#endif
