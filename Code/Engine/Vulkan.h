#pragma once

#ifdef VulkanBuild

#include "PCH.h"
#include "Media/Window.h"
#include "Basic/String.h"

typedef VkFormat GPUFormat;
#define GPU_FORMAT_R32G32B32_SFLOAT VK_FORMAT_R32G32B32_SFLOAT
#define GPU_FORMAT_R32_UINT VK_FORMAT_R32_UINT
#define GPU_FORMAT_D32_SFLOAT_S8_UINT VK_FORMAT_D32_SFLOAT_S8_UINT
#define GPU_FORMAT_R8G8B8A8_UNORM VK_FORMAT_R8G8B8A8_UNORM
#define GPU_FORMAT_D16_UNORM VK_FORMAT_D16_UNORM
#define GPU_FORMAT_UNDEFINED VK_FORMAT_UNDEFINED

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

void InitializeGPU(Window *win);

struct VulkanMemoryAllocation
{
	VkDeviceMemory vkMemory;
	s64 offset;
	s64 size;
	void *map;

	void Free();
};

extern VkDevice vkDevice;

struct GPUFence
{
	VkFence vkFence;

	bool Check();
	bool Wait(u64 timeout);
	void Free();
};

GPUFence NewGPUFence();
bool WaitForGPUFences(ArrayView<GPUFence> fs, bool waitAll, u64 timeout);
void ResetGPUFences(ArrayView<GPUFence> fs);

struct GPUSemaphore
{
	VkSemaphore vkSemaphore;

	void Free();
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

struct GPUFramebuffer
{
	VkFramebuffer vkFramebuffer;
};

struct GPURenderPass
{
	VkRenderPass vkRenderPass;
};

struct GPUPipeline
{
	VkPipeline vkPipeline;
};

enum GPUQueueType
{
	GPUGraphicsQueue,
	GPUTransferQueue,
	GPUComputeQueue,
	GPUQueueTypeCount
};

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
	GPUBufferType type;
	VulkanMemoryAllocation *memory;
	VkBuffer vkBuffer;

	void Free();
};

GPUBuffer NewGPUBuffer(GPUBufferType t, s64 size);

struct GPUImage
{
	VkImage vkImage;
	VulkanMemoryAllocation vkMemory;
	void *map;

	void Free();
};

GPUImage NewGPUImage(s64 w, s64 h, GPUFormat f, GPUImageLayout il, GPUImageUsageFlags uf, GPUSampleCount sc);

struct GPUCommandBuffer
{
	GPUQueueType type;
	VkCommandBuffer vkCommandBuffer;

	void BeginRenderPass(GPURenderPass rp, GPUFramebuffer fb);
	void EndRenderPass();
	void SetViewport(s64 w, s64 h);
	void SetScissor(s64 w, s64 h);
	void BindPipeline(GPUPipeline p);
	void BindVertexBuffer(GPUBuffer b, s64 bindPoint);
	void BindIndexBuffer(GPUBuffer b);
	//void BindDescriptorSets(GPUPipelineBindPoint pbp, GPUPipelineLayout pl, s64 firstSet, ArrayView<GPUDescriptorSet> dss);
	//void BindDescriptors(ArrayView<GPUDescriptors> ds);
	void DrawIndexedVertices(s64 numIndices, s64 firstIndex, s64 vertexOffset);
	void CopyBuffer(s64 size, GPUBuffer src, GPUBuffer dst, s64 srcOffset, s64 dstOffset);
	void CopyBufferToImage(GPUBuffer b, GPUImage i, u32 w, u32 h);
};

struct GPUFrameCommandBuffer : GPUCommandBuffer
{
	void Queue();
};

struct GPUAsyncCommandBuffer : GPUCommandBuffer
{
	void Queue(bool *signalOnCompletion);
};

GPUFence SubmitFrameGPUCommandBuffers(GPUQueueType t, ArrayView<GPUSemaphore> waitSems, ArrayView<GPUPipelineStageFlags> waitStages, ArrayView<GPUSemaphore> signalSems);
GPUFence SubmitAsyncGPUCommandBuffers();

struct GPUFrameStagingBuffer
{
	s64 size;
	VkBuffer vkBuffer;
	VkBuffer vkDestinationBuffer;

	void Flush();
};

GPUFrameStagingBuffer NewGPUFrameStagingBuffer(s64 size, GPUBuffer dst);

struct GPUAsyncStagingBuffer
{
	VulkanMemoryAllocation *memory;
	VkBuffer vkBuffer;
	VkBuffer vkDestinationBuffer;

	void Flush();
};

GPUAsyncStagingBuffer NewGPUAsyncStagingBuffer(s64 size, GPUBuffer dst);

void ClearGPUFrameResources(s64 frameIndex);

#endif
