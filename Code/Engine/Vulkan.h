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

struct GPUCommandBuffer
{
	VkCommandBuffer commandBuffer;
	VulkanQueueType queueType;

	void Free();
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
};

GPUCommandBuffer NewGPUCommandBuffer(s64 size, );
void QueueGPUCommandBuffer(GPUCommandBuffer cb, bool *signalOnCompletion);
void SubmitGPUCommandBuffers();

struct GPUBuffer
{
	VkBuffer buffer;

	void Free();
	void *Map(GPUSync sync);
	void FlushMap(void **map);
};

GPUBuffer NewGPUBuffer(GPUBufferUsage u, s64 size);

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
