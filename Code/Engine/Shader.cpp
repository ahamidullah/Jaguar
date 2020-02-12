#include "Shader.h"

void UpdateShaderDescriptors(Render_Context *context, GPU_Fence fence, s32 swapchainImageIndex, const Shader &shader, const ShaderMaterialParameters &material, const ShaderEntityParameters &entity) {
	void *stagingMemory;
	GPU_Buffer staging_buffer = CreateGPUStagingBuffer(context, sizeof(M4), &staging_memory);
	Copy_Memory(&update_infos[0].data.m4, staging_memory, sizeof(M4));
	GPU_Command_Buffer command_buffer = Render_API_Create_Command_Buffer(&context->api_context, context->thread_local_contexts[thread_index].command_pools[context->currentFrame]);
	Render_API_Record_Copy_Buffer_Command(&context->api_context, command_buffer, sizeof(M4), staging_buffer, sets->buffer, 0, 0x100 * swapchain_image_index);
	Render_API_End_Command_Buffer(&context->api_context, command_buffer);
	Render_API_Submit_Command_Buffers(&context->api_context, 1, &command_buffer, GPU_GRAPHICS_COMMAND_QUEUE, fence);
	Render_API_Update_Descriptor_Sets(&context->api_context, swapchain_image_index, sets->sets[0], sets->buffer);
}

void BindShaderEntityDescriptors() {
}

void BindShaderMaterialDescriptors() {
}
