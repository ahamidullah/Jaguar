#pragma once

#if 0

// @TODO: Switch all enums to match Vulkan name.

#if defined(USING_VULKAN_API)

#include "PCH.h"

#include "Code/Media/Window.h"

#include "Code/Basic/String.h"

constexpr bool usingVulkanAPI = true;

#if 0
#define GPU_MAX_FRAMES_IN_FLIGHT 2

#define GPU_MAX_MEMORY_HEAP_COUNT VK_MAX_MEMORY_HEAPS

typedef VkFlags GPUShaderStageFlags;
typedef VkShaderStageFlagBits GPUShaderStage;
#define GPU_VERTEX_SHADER_STAGE VK_SHADER_STAGE_VERTEX_BIT
#define GPU_FRAGMENT_SHADER_STAGE VK_SHADER_STAGE_FRAGMENT_BIT
#define GPU_COMPUTE_SHADER_STAGE VK_SHADER_STAGE_COMPUTE_BIT

typedef VkFlags GPUBufferUsageFlags;
typedef VkBufferUsageFlagBits GPUBufferUsage;
#define GPU_TRANSFER_DESTINATION_BUFFER VK_BUFFER_USAGE_TRANSFER_DST_BIT
#define GPU_TRANSFER_SOURCE_BUFFER VK_BUFFER_USAGE_TRANSFER_SRC_BIT
#define GPU_VERTEX_BUFFER VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
#define GPU_INDEX_BUFFER VK_BUFFER_USAGE_INDEX_BUFFER_BIT
#define GPU_UNIFORM_BUFFER VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT

typedef VkFlags GPUColorComponentFlags;
typedef VkColorComponentFlagBits GPUColorComponent;
#define GPU_COLOR_COMPONENT_RED VK_COLOR_COMPONENT_R_BIT
#define GPU_COLOR_COMPONENT_GREEN VK_COLOR_COMPONENT_G_BIT
#define GPU_COLOR_COMPONENT_BLUE VK_COLOR_COMPONENT_B_BIT
#define GPU_COLOR_COMPONENT_ALPHA VK_COLOR_COMPONENT_A_BIT

typedef VkFilter GPUFilter;
#define GPU_LINEAR_FILTER VK_FILTER_LINEAR
#define GPU_NEAREST_FILTER VK_FILTER_NEAREST
#define GPU_CUBIC_FILTER VK_FILTER_CUBIC_IMG

typedef VkFlags GPUImageUsageFlags;
typedef VkImageUsageFlagBits GPUImageUsage;
#define GPU_IMAGE_USAGE_TRANSFER_SRC VK_IMAGE_USAGE_TRANSFER_SRC_BIT
#define GPU_IMAGE_USAGE_TRANSFER_DST VK_IMAGE_USAGE_TRANSFER_DST_BIT
#define GPU_IMAGE_USAGE_SAMPLED VK_IMAGE_USAGE_SAMPLED_BIT
#define GPU_IMAGE_USAGE_STORAGE VK_IMAGE_USAGE_STORAGE_BIT
#define GPU_IMAGE_USAGE_COLOR_ATTACHMENT VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
#define GPU_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
#define GPU_IMAGE_USAGE_TRANSIENT_ATTACHMENT VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
#define GPU_IMAGE_USAGE_INPUT_ATTACHMENT VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
#define GPU_IMAGE_USAGE_SHADING_RATE_IMAGE_NV VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV
#define GPU_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_EXT VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT

typedef VkFlags GPUSampleCountFlags;
typedef VkSampleCountFlagBits GPUSampleCount;
#define GPU_SAMPLE_COUNT_1 VK_SAMPLE_COUNT_1_BIT
#define GPU_SAMPLE_COUNT_2 VK_SAMPLE_COUNT_2_BIT
#define GPU_SAMPLE_COUNT_4 VK_SAMPLE_COUNT_4_BIT
#define GPU_SAMPLE_COUNT_8 VK_SAMPLE_COUNT_8_BIT
#define GPU_SAMPLE_COUNT_16 VK_SAMPLE_COUNT_16_BIT
#define GPU_SAMPLE_COUNT_32 VK_SAMPLE_COUNT_32_BIT
#define GPU_SAMPLE_COUNT_64 VK_SAMPLE_COUNT_64_BIT

typedef VkFlags GPUPipelineStageFlags;
typedef VkPipelineStageFlagBits GPUPipelineStage;
#define GPU_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT

typedef VkBlendFactor GPUBlendFactor;
#define GPU_BLEND_FACTOR_SRC_ALPHA VK_BLEND_FACTOR_SRC_ALPHA
#define GPU_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
#define GPU_BLEND_FACTOR_ONE VK_BLEND_FACTOR_ONE
#define GPU_BLEND_FACTOR_ZERO VK_BLEND_FACTOR_ZERO

enum GPUBlendOperation
{
	GPU_BLEND_OP_ADD = VK_BLEND_OP_ADD,
};

enum GPUVertexInputRate
{
	GPU_VERTEX_INPUT_RATE_VERTEX = VK_VERTEX_INPUT_RATE_VERTEX,
	GPU_VERTEX_INPUT_RATE_INSTANCE = VK_VERTEX_INPUT_RATE_INSTANCE,
};

typedef VkFormat GPUFormat;
#define GPU_FORMAT_R32G32B32_SFLOAT VK_FORMAT_R32G32B32_SFLOAT
#define GPU_FORMAT_R32_UINT VK_FORMAT_R32_UINT
#define GPU_FORMAT_D32_SFLOAT_S8_UINT VK_FORMAT_D32_SFLOAT_S8_UINT
#define GPU_FORMAT_R8G8B8A8_UNORM VK_FORMAT_R8G8B8A8_UNORM
#define GPU_FORMAT_D16_UNORM VK_FORMAT_D16_UNORM
#define GPU_FORMAT_UNDEFINED VK_FORMAT_UNDEFINED

typedef VkDescriptorType GPUDescriptorType;
#define GPU_DESCRIPTOR_TYPE_UNIFORM_BUFFER VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
#define GPU_DESCRIPTOR_TYPE_DYNAMIC_UNIFORM_BUFFER VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
#define GPU_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
#define GPU_DESCRIPTOR_TYPE_SAMPLER VK_DESCRIPTOR_TYPE_SAMPLER
#define GPU_DESCRIPTOR_TYPE_SAMPLED_IMAGE VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE

enum GPUPipelineTopology
{
	GPU_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	GPU_PRIMITIVE_TOPOLOGY_LINE_LIST = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
};

enum GPUCompareOperation
{
	GPU_COMPARE_OP_NEVER = VK_COMPARE_OP_NEVER,
	GPU_COMPARE_OP_LESS = VK_COMPARE_OP_LESS,
	GPU_COMPARE_OP_EQUAL = VK_COMPARE_OP_EQUAL,
	GPU_COMPARE_OP_LESS_OR_EQUAL = VK_COMPARE_OP_LESS_OR_EQUAL,
	GPU_COMPARE_OP_GREATER = VK_COMPARE_OP_GREATER,
	GPU_COMPARE_OP_NOT_EQUAL = VK_COMPARE_OP_NOT_EQUAL,
	GPU_COMPARE_OP_GREATER_OR_EQUAL = VK_COMPARE_OP_GREATER_OR_EQUAL,
	GPU_COMPARE_OP_ALWAYS = VK_COMPARE_OP_ALWAYS,
};

enum GPUDynamicPipelineState
{
	GPU_DYNAMIC_PIPELINE_STATE_VIEWPORT = VK_DYNAMIC_STATE_VIEWPORT,
	GPU_DYNAMIC_PIPELINE_STATE_SCISSOR = VK_DYNAMIC_STATE_SCISSOR,
	GPU_DYNAMIC_PIPELINE_STATE_LINE_WIDTH = VK_DYNAMIC_STATE_LINE_WIDTH,
	GPU_DYNAMIC_PIPELINE_STATE_DEPTH_BIAS = VK_DYNAMIC_STATE_DEPTH_BIAS,
	GPU_DYNAMIC_PIPELINE_STATE_BLEND_CONSTANTS = VK_DYNAMIC_STATE_BLEND_CONSTANTS,
	GPU_DYNAMIC_PIPELINE_STATE_DEPTH_BOUNDS = VK_DYNAMIC_STATE_DEPTH_BOUNDS,
	GPU_DYNAMIC_PIPELINE_STATE_STENCIL_COMPARE_MASK = VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
	GPU_DYNAMIC_PIPELINE_STATE_STENCIL_WRITE_MASK = VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
	GPU_DYNAMIC_PIPELINE_STATE_STENCIL_REFERENCE = VK_DYNAMIC_STATE_STENCIL_REFERENCE,
};

typedef VkImageLayout GPUImageLayout;
#define GPU_IMAGE_LAYOUT_UNDEFINED VK_IMAGE_LAYOUT_UNDEFINED
#define GPU_IMAGE_LAYOUT_GENERAL VK_IMAGE_LAYOUT_GENERAL
#define GPU_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
#define GPU_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
#define GPU_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
#define GPU_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
#define GPU_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
#define GPU_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
#define GPU_IMAGE_LAYOUT_PREINITIALIZED VK_IMAGE_LAYOUT_PREINITIALIZED
#define GPU_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL
#define GPU_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL
#define GPU_IMAGE_LAYOUT_PRESENT_SRC VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
#define GPU_IMAGE_LAYOUT_SHARED_PRESENT VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR
#define GPU_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV
#define GPU_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT

typedef VkPipelineBindPoint GPUPipelineBindPoint;
#define GPU_GRAPHICS_PIPELINE_BIND_POINT VK_PIPELINE_BIND_POINT_GRAPHICS
#define GPU_COMPUTE_PIPELINE_BIND_POINT VK_PIPELINE_BIND_POINT_COMPUTE

typedef VkPipelineStageFlags GPUPipelineStageFlags;
#define GPU_PIPELINE_STAGE_TOP_OF_PIPE VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
#define GPU_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT

enum GPUSamplerAddressMode
{
	GPU_SAMPLER_ADDRESS_MODE_REPEAT = VK_SAMPLER_ADDRESS_MODE_REPEAT,
	GPU_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
	GPU_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
	GPU_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
	GPU_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE,
};

enum GPUBorderColor
{
	GPU_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
	GPU_BORDER_COLOR_INT_TRANSPARENT_BLACK = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK,
	GPU_BORDER_COLOR_FLOAT_OPAQUE_BLACK = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
	GPU_BORDER_COLOR_INT_OPAQUE_BLACK = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
	GPU_BORDER_COLOR_FLOAT_OPAQUE_WHITE = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
	GPU_BORDER_COLOR_INT_OPAQUE_WHITE = VK_BORDER_COLOR_INT_OPAQUE_WHITE,
};

typedef VkComponentSwizzle GPUSwizzle;
#define GPU_SWIZZLE_MAPPING_IDENTITY VK_COMPONENT_SWIZZLE_IDENTITY

typedef VkImageAspectFlags GPUImageAspectFlags;
#define GPU_IMAGE_ASPECT_COLOR VK_IMAGE_ASPECT_COLOR_BIT
#define GPU_IMAGE_ASPECT_DEPTH VK_IMAGE_ASPECT_DEPTH_BIT

typedef VkImageViewType GPUImageViewType;
#define GPU_IMAGE_VIEW_TYPE_2D VK_IMAGE_VIEW_TYPE_2D

struct GPUSwizzleMapping
{
	GPUSwizzle r;
	GPUSwizzle g;
	GPUSwizzle b;
	GPUSwizzle a;
};

#if 0
struct GfxImageSubresourceRange
{
	GfxImageAspectFlags aspectMask;
	u32 baseMipLevel;
	u32 levelCount;
	u32 baseArrayLayer;
	u32 layerCount;
};
#endif

struct VulkanSamplerFilter
{
	VkFilter min;
	VkFilter mag;
	VkSamplerMipmapMode mipmap;
};

//
// Internal: only accessed through the GPU.cpp layer.
//

typedef VkImage GPUInternalImage;
typedef VkBuffer GPUInternalBuffer;
typedef VkCommandBuffer GPUInternalCommandBuffer;
typedef VkQueue GPUInternalCommandQueue;
typedef VkCommandPool GPUInternalCommandPool;
typedef VkDeviceMemory GPUInternalMemory;
typedef VkMemoryRequirements GPUInternalMemoryRequirements;

void InitializeGPUInternal(Window *window);
GPUInternalCommandBuffer NewGPUInternalCommandBuffer(GPUInternalCommandPool cp);
void SubmitGPUInternalCommandBuffers(GPUInternalCommandQueue cq, GPUSubmitInfo si, GPUFence f);
void FreeGPUInternalCommandBuffers(GPUInternalCommandPool cp, s64 n, GPUInternalCommandBuffer *cbs);
void EndGPUInternalCommandBuffer(GPUInternalCommandBuffer cb);
GPUInternalCommandQueue GetGPUInternalCommandQueue(GPUInternalCommandQueueType t);
GPUInternalCommandPool NewGPUInternalCommandPool(GPUInternalCommandQueueType qt);
void ResetGPUInternalCommandPool(GPUInternalCommandPool cp);
GPUInternalBuffer NewGPUInternalBuffer(s64 size, GPUBufferUsageFlags uf);
void DestroyGPUInternalBuffer(GPUInternalBuffer b);
void RecordGPUInteralCopyBufferCommand(GPUCommandBuffer cb, s64 size, GPUInternalBuffer src, GPUInternalBuffer dst, s64 srcOffset, s64 dstOffset);
GPUInternalMemoryRequirements GetGPUInternalBufferMemoryRequirements(GPUInternalBuffer b);
bool AllocateGPUInternalMemory(s64 size, GPUMemoryType mt, GPUInternalMemory *m);
void *MapGPUInternalMemory(GPUInternalMemory memory, s64 size, s64 offset);
GPUInternalImage NewGPUInternalImage(u32 w, u32 h, GPUFormat f, GPUImageLayout il, GPUImageUsageFlags uf, GPUSampleCount sc);
GPUInternalMemoryRequirements GetGPUInternalImageMemoryRequirements(GPUInternalImage i);

//
// External: accessible anywhere.
//

typedef VkSemaphore GPUSemaphore;
typedef VkDescriptorSetLayout GPUDescriptorSetLayout;
typedef VkDescriptorSet GPUDescriptorSet;
typedef VkFence GPUFence;
typedef VkSampler GPUSampler;
typedef VkShaderModule GPUShaderModule;
typedef VkRenderPass GPURenderPass;
typedef VkDescriptorPool GPUDescriptorPool;
typedef VkFramebuffer GPUFramebuffer;
typedef VkSwapchainKHR GPUSwapchain;
typedef VkImageView GPUImageView;
typedef VulkanSamplerFilter GPUSamplerFilter;
typedef VkPipeline GPUPipeline;
typedef VkPipelineLayout GPUPipelineLayout;
typedef VkDeviceSize GPUSize;
typedef VkImageSubresourceRange GPUImageSubresourceRange;

Array<GPUMemoryHeapInfo> GPUMemoryInfo();
void PrintGPUMemoryInfo();
s64 GPUMemoryHeapIndex(GPUMemoryType memoryType);
void BindGPUBufferMemory(GPUBuffer b, GPUMemory m, s64 memOffset);
s64 GPUBufferImageGranularity();
GPUFormat GPUSurfaceFormat();
GPUShaderModule GPUCreateShaderModule(GPUShaderStage stage, String spirv);
GPUFence NewGPUFence(bool startSignalled);
bool WasGPUFenceSignalled(GPUFence f);
void WaitForGPUFences(ArrayView<GPUFence> fs, bool waitAll, u64 timeout);
void ResetGPUFences(ArrayView<GPUFence> fs);
GPUSwapchain NewGPUSwapchain();
u32 AcquireNextGPUSwapchainImage(GPUSwapchain sc, GPUSemaphore s);
u32 GetGPUSwapchainImageCount(GPUSwapchain sc);
void GetGPUSwapchainImages(GPUSwapchain sc, Array<GPUImage> *out);
void PresentGPUSwapchainImage(GPUSwapchain sc, u32 index);
GPUDescriptorPool NewGPUDescriptorPool();
GPUFramebuffer NewGPUFramebuffer(GPURenderPass rp, u32 w, u32 h, ArrayView<GPUImageView> attachments);
GPUSemaphore NewGPUSemaphore();
GPUDescriptorSetLayout NewGPUDescriptorSetLayout(u32 numBindings, DescriptorSetBindingInfo *bis);
void UpdateGPUDescriptorSets(GPUDescriptorSet ds, GPUBuffer b, GPUDescriptorType dt, u32 binding, s64 offset, s64 range);
void NewGPUDescriptorSets(GPUDescriptorPool dp, GPUDescriptorSetLayout dsl, ArrayView<GPUDescriptorSet> dss);
GPUPipeline NewGPUPipeline(GPUPipelineDescription pd);
GPUPipelineLayout NewGPUPipelineLayout(ArrayView<GPUDescriptorSetLayout> dsls);
GPURenderPass TEMPORARY_Render_API_Create_Render_Pass();
void BindGPUImageMemory(GPUImage i, GPUMemory m, s64 offset);
void TransitionGPUImageLayout(GPUCommandBuffer cb, GPUImage i, GPUFormat f, GPUImageLayout old, GPUImageLayout new);
GPUImageView NewGPUImageView(GPUImage image, GPUImageViewType ivt, GPUFormat f, GPUSwizzleMapping sm, GPUImageSubresourceRange isr);
void GPURecordBeginRenderPassCommand(GPUCommandBuffer cb, GPURenderPass rp, GPUFramebuffer fb);
void GPURecordSetViewportCommand(GPUCommandBuffer cb, s64 w, s64 h);
void GPURecordSetScissorCommand(GPUCommandBuffer cb, u32 width, u32 height);
void GPURecordBindPipelineCommand(GPUCommandBuffer cb, GPUPipeline p);
void GPURecordBindVertexBufferCommand(GPUCommandBuffer cb, GPUBuffer b);
void GPURecordBindIndexBufferCommand(GPUCommandBuffer cb, GPUBuffer b);
void GPURecordDrawIndexedVerticesCommand(GPUCommandBuffer cb, s64 numIndices, s64 firstIndex, s64 vertexOffset);
void GPURecordEndRenderPassCommand(GPUCommandBuffer cb);
void GPURecordCopyBufferToImageCommand(GPUCommandBuffer cb, GPUBuffer b, GPUImage i, u32 w, u32 h);
void GPURecordBindDescriptorSetsCommand(GPUCommandBuffer cb, GPUPipelineBindPoint pbp, GPUPipelineLayout pl, s64 firstSet, ArrayView<GPUDescriptorSet> dss);
void GPURecordBindPipelineCommand(GPUCommandBuffer cb, GPUPipeline p);
void GPUPresentSwapchainImage(GPUSwapchain sc, u32 index, Array<GPUSemaphore> wait);
#endif

typedef VkPipelineStageFlags GPUPipelineStageFlags;
#define GPUTopStage VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
#define GPUColorAttachmentOutputStage VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT

#endif

struct GPUFence
{
	VkFence vkFence;

	bool Check();
	bool Wait(u64 timeout);
	void Free();
};

GPUFence NewGPUFence(bool startSignalled);
bool WaitForGPUFences(ArrayView<GPUFence> fs, bool waitAll, u64 timeout);
void ResetGPUFences(ArrayView<GPUFence> fs);

struct GPUSemaphore
{
	VkSemaphore vkSemaphore;

	void Free();
};

struct GPUSync
{
	GPUFramesync,
	GPUAsync,
};

struct GPUMemoryHeapInfo
{
	s64 usage;
	s64 budget;
};

struct GPUShader
{
	VkShaderModule shader;
};

GPUShader NewShader();

enum GPUQueueType
{
	GPUGraphicsQueue,
	GPUTransferQueue,
	GPUComputeQueue,
	GPUCommandQueueCount
};

template <GPUSync S>
struct GPUCommandBuffer
{
	GPUQueueType type;
	VkCommandBuffer vkCommandBuffer;

	void BeginRenderPass(GPURenderPass rp, GPUFramebuffer fb);
	void EndRenderPass();
	void SetViewport(s64 w, s64 h);
	void SetScissor(u32 w, u32 h);
	void BindPipeline(GPUPipeline p);
	void BindVertexBuffer(GPUBuffer b);
	void BindIndexBuffer(GPUBuffer b);
	//void BindDescriptorSets(GPUPipelineBindPoint pbp, GPUPipelineLayout pl, s64 firstSet, ArrayView<GPUDescriptorSet> dss);
	void BindDescriptors(ArrayView<GPUDescriptors> ds);
	void DrawIndexedVertices(s64 numIndices, s64 firstIndex, s64 vertexOffset);
	void CopyBuffer(s64 size, GPUBuffer src, GPUBuffer dst, s64 srcOffset, s64 dstOffset);
	void CopyBufferToImage(GPUBuffer b, GPUImage i, u32 w, u32 h);
	void Queue();
};

template <GPUSync S>
GPUCommandBuffer<S> NewGPUCommandBuffer(GPUQueueType t)
{
	auto MakeCB = [](GPUQueueType t, VkCommandPool p) -> VkCommandBuffer
	{
    	auto ai = VkCommandBufferAllocateInfo
    	{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
			.commandPool = p,
    	};
    	auto cb = VkCommandBuffer{};
    	vkAllocateCommandBuffers(vkDevice, &ai, &cb);
		auto bi = VkCommandBufferBeginInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		};
		vkBeginCommandBuffer(cb, &bi);
		return cb;
	};
	auto cb = GPUCommandBuffer<S>
	{
		.type = t,
	};
    if constexpr (S == GPUFramesync)
    {
    	cb.vkCommandBuffer = vkThreadLocal[ThreadIndex()].frameCommandBufferPools[t].Get();
    	if (!cb.vkCommandBuffer)
    	{
    		cb.vkCommandBuffer = MakeCB(t, vkThreadLocal[ThreadIndex()].frameCommandPools[FrameIndex()]);
    	}
    }
	else
	{
		vkThreadLocal[ThreadIndex()].asyncLock.Lock();
		Defer(vkThreadLocal[ThreadIndex()].asyncLock.Unlock());
		cb.vkCommandBuffer = vkThreadLocal[ThreadIndex()].asyncCommandBufferPools[t].Get();
		if (!cb.vkCommandBuffer)
		{
			cb.vkCommandBuffer = MakeCB(t, vkThreadLocal[ThreadIndex()].asyncCommandPools[t]);
		}
	}
	DisableJobSwitching();
	return cb;
}

template <GPUSync S>
void GPUCommandBuffer<S>::BeginRenderPass(GPURenderPass rp, GPUFramebuffer fb)
{
	auto clearColor = VkClearValue
	{
		.color.float32 = {0.04f, 0.19f, 0.34f, 1.0f},
	};
	auto clearDepthStencil = VkClearValue
	{
		.depth = 1.0f,
		.stencil = 0.0f,
	}
	auto clearValues = MakeStaticArray<VkClearValue>(clearColor, clearDepthStencil);
	auto bi = VkRenderPassBeginInfo
	{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = rp,
		.framebuffer = fb,
		.renderArea =
		{
			.offset = {0, 0},
			.extent = {(u32)RenderWidth(), (u32)RenderHeight()},
		},
		.clearValueCount = clearValues.Count(),
		.pClearValues = clearValues.elements,
	};
	vkCmdBeginRenderPass(this->vkCommandBuffer, &bi, VK_SUBPASS_CONTENTS_INLINE);
}

template <GPUSync S>
void GPUCommandBuffer<S>::EndRenderPass()
{
	vkCmdEndRenderPass(this->vkCommandBuffer);
}

template <GPUSync S>
void GPUCommandBuffer<S>::SetViewport(s64 w, s64 h)
{
	auto v = VkViewport
	{
		.x = 0.0f,
		.y = 0.0f,
		.width = (f32)w,
		.height = (f32)h,
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};
	vkCmdSetViewport(this->vkCommandBuffer, 0, 1, &v);
}

template <GPUSync S>
void GPUCommandBuffer<S>::SetScissor(s64 w, s64 h)
{
	auto scissor = VkRect2D
	{
		.extent = {w, h},
	};
	vkCmdSetScissor(this->vkCommandBuffer, 0, 1, &scissor);
}

template <GPUSync S>
void GPUCommandBuffer<S>::BindPipeline(GPUPipeline p)
{
	vkCmdBindPipeline(this->vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p);
}

template <GPUSync S>
void GPUCommandBuffer<S>::BindVertexBuffer(GPUBuffer b, s64 bindPoint)
{
	auto offset = VkDeviceSize{0};
	vkCmdBindVertexBuffers(this->vkCommandBuffer, bindPoint, 1, &b, &offset);
}

template <GPUSync S>
void GPUCommandBuffer<S>::BindIndexBuffer(GPUBuffer b)
{
	vkCmdBindIndexBuffer(this->vkCommandBuffer, b, 0, VK_INDEX_TYPE_UINT32);
}

template <GPUSync S>
void GPUCommandBuffer<S>::BindDescriptors(ArrayView<GPUDescriptors> ds)
{
	// @TODO
}

template <GPUSync S>
void GPUCommandBuffer<S>::DrawIndexedVertices(s64 numIndices, s64 firstIndex, s64 vertexOffset)
{
	vkCmdDrawIndexed(this->vkCommandBuffer, numIndices, 1, firstIndex, vertexOffset, 0);
}

template <GPUSync S>
void GPUCommandBuffer<S>::CopyBuffer(s64 size, GPUBuffer src, GPUBuffer dst, s64 srcOffset, s64 dstOffset)
{
	auto c = VkBufferCopy
	{
		.srcOffset = srcOffset,
		.dstOffset = dstOffset,
		.size = size,
	};
	vkCmdCopyBuffer(this->vkCommandBuffer, src, dst, 1, &c);
}

template <GPUSync S>
void GPUCommandBuffer<S>::CopyBufferToImage(GPUBuffer b, GPUImage i, u32 w, u32 h)
{
	auto region = VkBufferImageCopy
	{
		.bufferOffset = 0,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,
		.imageSubresource =
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
		.imageOffset = {},
		.imageExtent =
		{
			w,
			h,
			1,
		},
	};
	vkCmdCopyBufferToImage(this->vkCommandBuffer, b, i, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

template <GPUSync S>
void GPUCommandBuffer<S>::End()
{
	VkCheck(vkEndCommandBuffer(this->vkCommandBuffer));
	//EnableJobSwitching();
}

template <GPUSync S>
void GPUCommandBuffer<S>::Queue(bool *signalOnCompletion)
{
	if (S == GPUFramesync)
	{
		vkThreadLocal[CurrentThread()].frameQueuedCommandBuffers.Append(this);
	}
	else
	{
		vkThreadLocal[CurrentThread()].asyncLock.Lock();
		Defer(vkThreadLocal[CurrentThread()].asyncLock.Unlock());
		vkThreadLocal[CurrentThread()].asyncQueuedCommandBuffers[this->type].Append(this->vkBuffer);
		vkThreadLocal[CurrentThread()].asyncSignals[this->type].Append(signalOnCompletion);
	}
}

GPUFence SubmitQueuedFrameGPUCommandBuffers(GPUQueueType t, ArrayView<GPUSemaphore> waitSems, ArrayView<GPUPipelineStageFlags> waitStages, ArrayView<GPUSemaphore> signalSems);
GPUFence SubmitQueuedAsyncGPUCommandBuffers(GPUQueueType t);

enum GPUBufferType
{
	GPUVertexBuffer,
	GPUIndexBuffer,
	GPUUniformBuffer,
	GPUTransferBuffer,
	GPUBufferTypeCount
};

struct GPUBuffer
{
	VulkanMemoryAllocation memory;
	VkBuffer vkBuffer;

	void Free();
};

GPUBuffer NewGPUBuffer(GPUBufferType t, s64 size);

template <GPUSync S>
struct GPUStagingBuffer
{
	VulkanMemoryAllocation memory;
	VkBuffer buffer;
	VkBuffer destinationBuffer;

	void Flush();
};

template <GPUSync S>
GPUStagingBuffer<S> NewGPUStagingBuffer(s64 size, GPUBuffer dest)
{
	auto MakeBuf = [](VulkanMemoryAllocation mem, s64 size) -> VkBuffer
	{
		auto ci = VkBufferCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = size,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		};
		auto b = VkBuffer{};
		VkCheck(vkCreateBuffer(vkDevice, &ci, NULL, &b));
		VkCheck(vkBindBufferMemory(vkDevice, b, mem.vkDeviceMemory, 0));
		return b;
	};
	auto sb = GPUStagingBuffer
	{
		.destinationBuffer = dest,
	};
	if constexpr (S == GPUFramesync)
	{
		sb.memory = vkThreadLocal[ThreadIndex()].frameCPUToGPURingAllocator[FrameIndex()].Allocate(size);
		sb.buffer = vkThreadLocal[ThreadIndex()].frameBufferPool[FrameIndex()][GPUTransferBuffer].Get();
		if (!sb.buffer)
		{
			sb.buffer = MakeBuf(sb.memory, size);
		}
	}
	else
	{
		sb.memory = vkCPUToGPUBlockAllocator.Allocate(size);
		sb.buffer = vkThreadLocal[ThreadIndex()].asyncBufferPool.Get();
		if (!sb.buffer)
		{
			sb.buffer = MakeBuf(sb.memory, size);
		}
	}
	return sb;
}

template <GPUSync S>
void GPUStagingBuffer<S>::Flush()
{
	auto cb = NewGPUCommandBuffer<S>(GPUTransferCommandBuffer);
	cb.CopyBuffer(this->buffer, this->destinationBuffer, this->memory.size, 0, 0);
	if constexpr (S == GPUFramesync)
	{
		cb.Queue(NULL);
	}
	else
	{
		vkThreadLocal[ThreadIndex()].asyncTransferLock.Lock();
		vkThreadLocal[ThreadIndex()].asyncStagingSignals.Resize(vkThreadLocal[ThreadIndex()].asyncStagingSignals.count + 1);
		cb.Queue(&vkThreadLocal[ThreadIndex()].asyncSignals[vkThreadLocal[ThreadIndex()].asyncSignals.count - 1]);
		vkThreadLocal[ThreadIndex()].asyncPendingStagingBuffers.Append(this);
	}
}

struct GPUImage
{
	VkImage image;
	VulkanMemoryAllocation vkMemory;
	void *mappedMemory;

	void Free();
};

GPUImage NewGPUImage(s64 w, s64 h, GPUFormat f, GPUImageLayout il, GPUImageUsageFlags uf, GPUSampleCount sc, GPUMemoryType mt, GPULifetime lt);

struct GPUFramebuffer
{
};

void ClearGPUFrameResources(s64 frameIndex);

#if 0
#if 0
#define GFX_ERROR_OBJECT 0

#define GFX_DESCRIPTOR_SET_COUNT 1

typedef struct Render_API_Context
{
	VkPhysicalDevice physical_device;
	VkDevice device;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debug_messenger;
	VkSurfaceKHR window_surface;
	VkSurfaceFormatKHR window_surface_format;
	u32 graphics_queue_family;
	u32 present_queue_family;
	VkQueue graphics_queue;
	VkQueue present_queue;
	VkPresentModeKHR present_mode;
	VkDescriptorSetLayout descriptor_set_layouts[GFX_DESCRIPTOR_SET_COUNT];

	//u32 descriptor_set_count;
	//Vulkan_Descriptor_Set_Layouts descriptor_set_layouts;
	//Vulkan_Descriptor_Sets descriptor_sets;
	//VkSwapchainKHR swapchain;

	VkDeviceSize minimum_uniform_buffer_offset_alignment; // Any uniform or dynamic uniform buffer's offset inside a Vulkan memory block must be a multiple of this byte count.
	VkDeviceSize maximum_uniform_buffer_size; // Maximum size of any uniform buffer (including dynamic uniform buffers). @TODO: Move to sizes struct?
	VkSemaphore image_available_semaphores[GFX_MAX_FRAMES_IN_FLIGHT];
	VkSemaphore render_finished_semaphores[GFX_MAX_FRAMES_IN_FLIGHT];
} Render_API_Context;
#endif

#else

constexpr bool usingVulkanAPI = false;

#endif
#endif
