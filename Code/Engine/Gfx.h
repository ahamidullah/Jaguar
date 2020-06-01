#pragma once

struct GfxSubmitInfo
{
	Array<GfxCommandBuffer> commandBuffers;
	Array<GfxPipelineStageFlags> waitStages;
    Array<GfxSemaphore> waitSemaphores;
    Array<GfxSemaphore> signalSemaphores;
};

enum GfxCommandQueueType
{
	GFX_GRAPHICS_COMMAND_QUEUE,
	GFX_COMPUTE_COMMAND_QUEUE,
	GFX_TRANSFER_COMMAND_QUEUE,

	GFX_COMMAND_QUEUE_COUNT
};

enum GfxMemoryType
{
	GFX_GPU_ONLY_MEMORY,
	GFX_CPU_TO_GPU_MEMORY,
	GFX_GPU_TO_CPU_MEMORY,

	GFX_MEMORY_TYPE_COUNT,
};

// @TODO: Rename me.
struct DescriptorSetBindingInfo
{
	u32 binding;
	GfxDescriptorType descriptorType;
	u32 descriptorCount;
	GfxShaderStageFlags stageFlags;
};

struct GfxFramebufferAttachmentColorBlendDescription
{
	bool enable_blend;
	GfxBlendFactor source_color_blend_factor;
	GfxBlendFactor destination_color_blend_factor;
	GfxBlendOperation color_blend_operation;
	GfxBlendFactor source_alpha_blend_factor;
	GfxBlendFactor destination_alpha_blend_factor;
	GfxBlendOperation alpha_blend_operation;
	GfxColorComponentFlags color_write_mask;
};

struct GfxPipelineVertexInputAttributeDescription
{
	GfxFormat format;
	u32 binding;
	u32 location;
	u32 offset;
};

struct GfxPipelineVertexInputBindingDescription
{
	u32 binding;
	u32 stride;
	GfxVertexInputRate input_rate;
};

struct GfxPipelineDescription
{
	GfxPipelineLayout layout;
	GfxPipelineTopology topology;
	f32 viewport_width;
	f32 viewport_height;
	u32 scissor_width;
	u32 scissor_height;
	GfxCompareOperation depth_compare_operation;
	u32 framebuffer_attachment_color_blend_count;
	GfxFramebufferAttachmentColorBlendDescription *framebuffer_attachment_color_blend_descriptions;
	u32 vertex_input_attribute_count;
	GfxPipelineVertexInputAttributeDescription *vertex_input_attribute_descriptions;
	u32 vertex_input_binding_count;
	GfxPipelineVertexInputBindingDescription *vertex_input_binding_descriptions;
	u32 dynamic_state_count;
	GfxDynamicPipelineState *dynamic_states;
	Array<GfxShaderStage> shaderStages;
	Array<GfxShaderModule> shaderModules;
	GfxRenderPass render_pass;
	bool enable_depth_bias;
};
