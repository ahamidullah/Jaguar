#pragma once

#ifdef VulkanBuild

#include "PCH.h"
#include "Math.h"
#include "Media/Window.h"
#include "Basic/String.h"

typedef VkFormat GPUFormat;
#define GPUFormatD32SfloatS8Uint VK_FORMAT_D32_SFLOAT_S8_UINT

typedef VkImageLayout GPUImageLayout;
#define GPUImageLayoutUndefined VK_IMAGE_LAYOUT_UNDEFINED

typedef VkImageAspectFlags GPUImageAspectFlags;
#define GPUImageAspectColor VK_IMAGE_ASPECT_COLOR_BIT
#define GPUImageAspectDepth VK_IMAGE_ASPECT_DEPTH_BIT

typedef VkImageViewType GPUImageViewType;
#define GPUImageViewType2D VK_IMAGE_VIEW_TYPE_2D

typedef VkFlags GPUImageUsageFlags;
typedef VkImageUsageFlagBits GPUImageUsage;
#define GPUImageUsageDepthStencilAttachment VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT

typedef VkComponentSwizzle GPUSwizzle;
#define GPUSwizzleMappingIdentity VK_COMPONENT_SWIZZLE_IDENTITY

typedef VkFlags GPUSampleCountFlags;
typedef VkSampleCountFlagBits GPUSampleCount;
#define GPUSampleCount1 VK_SAMPLE_COUNT_1_BIT

typedef VkFlags GPUPipelineStageFlags;
typedef VkPipelineStageFlagBits GPUPipelineStage;
#define GPU_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT

typedef VkIndexType GPUIndexType;
#define GPUIndexTypeUint16 VK_INDEX_TYPE_UINT16
#define GPUIndexTypeUint32 VK_INDEX_TYPE_UINT32

void InitializeGPU(Window *win);

enum GPUShaderID
{
	GPUModelShaderID,
	GPUShaderIDCount
};

void GPUCompileShaderFromFile(GPUShaderID id, String path, bool *err);

void GPUBeginFrame();
void GPUEndFrame();

struct GPUMemoryHeapInfo
{
	u64 usage;
	u64 budget;
};

GPUMemoryHeapInfo GPUMemoryUsage();
void LogGPUMemoryInfo();

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

// @TODO: Get rid of the memory allocation in the struct?

struct GPUBuffer
{
	VulkanMemoryAllocation *memory;
	VkBuffer vkBuffer;

	void Free();
};

GPUBuffer NewGPUVertexBuffer(s64 size);
GPUBuffer NewGPUIndexBuffer(s64 size);

struct GPUBufferView
{
	VkBufferView vkBufferView;

	void Free();
};

GPUBufferView NewGPUBufferView(GPUBuffer src, GPUFormat fmt, s64 offset, s64 range);

struct GPUImage
{
	VkImage vkImage;
	VulkanMemoryAllocation *memory;

	void Free();
};

GPUImage NewGPUImage(s64 w, s64 h, GPUFormat f, GPUImageLayout il, GPUImageUsageFlags uf, GPUSampleCount sc);

struct GPUImageView
{
	VkImageView vkImageView;
};

struct GPUSwizzleMapping
{
	GPUSwizzle r;
	GPUSwizzle g;
	GPUSwizzle b;
	GPUSwizzle a;
};

struct GPUImageSubresourceRange
{
	GPUImageAspectFlags aspectMask;
	u32 baseMipLevel;
	u32 levelCount;
	u32 baseArrayLayer;
	u32 layerCount;
};

GPUImageView NewGPUImageView(GPUImage src, GPUImageViewType t, GPUFormat f, GPUSwizzleMapping sm, GPUImageSubresourceRange isr);

struct GPUTexture
{
	VkImage vkImage;
	VkImageView vkImageView;
};

struct GPUSampler
{
	VkSampler vkSampler;
};

struct GPUFramebuffer
{
	u64 id;
	s64 w;
	s64 h;
	Array<GPUImageView> attachments;
};

GPUFramebuffer GPUDefaultFramebuffer();

struct GPUCommandBuffer
{
	VkCommandBuffer vkCommandBuffer;

	void BeginRender(GPUShaderID id, GPUFramebuffer fb);
	void EndRender();
	void SetViewport(s64 w, s64 h);
	void SetScissor(s64 w, s64 h);
	void BindVertexBuffer(GPUBuffer b, s64 bindPoint);
	void BindIndexBuffer(GPUBuffer b, GPUIndexType t);
	void DrawIndexedVertices(s64 numIndices, s64 firstIndex, s64 vertexOffset);
	void CopyBuffer(s64 size, GPUBuffer src, GPUBuffer dst, s64 srcOffset, s64 dstOffset);
	void CopyBufferToImage(GPUBuffer b, GPUImage i, u32 w, u32 h);
};

struct GPUFrameGraphicsCommandBuffer : GPUCommandBuffer
{
	void Queue();
};

GPUFrameGraphicsCommandBuffer NewGPUFrameGraphicsCommandBuffer();

struct GPUFrameTransferCommandBuffer : GPUCommandBuffer
{
	void Queue();
};

GPUFrameTransferCommandBuffer NewGPUFrameTransferCommandBuffer();

struct GPUFrameComputeCommandBuffer : GPUCommandBuffer
{
	void Queue();
};

GPUFrameComputeCommandBuffer NewGPUFrameComputeCommandBuffer();

struct GPUAsyncGraphicsCommandBuffer : GPUCommandBuffer
{
	void Queue(bool *signalOnCompletion);
};

GPUAsyncGraphicsCommandBuffer NewGPUAsyncGraphicsCommandBuffer();

struct GPUAsyncTransferCommandBuffer : GPUCommandBuffer
{
	void Queue(bool *signalOnCompletion);
};

GPUAsyncTransferCommandBuffer NewGPUAsyncTransferCommandBuffer();

struct GPUAsyncComputeCommandBuffer : GPUCommandBuffer
{
	void Queue(bool *signalOnCompletion);
};

GPUAsyncComputeCommandBuffer NewGPUAsyncComputeCommandBuffer();

GPUFence GPUSubmitFrameGraphicsCommandBuffers();
GPUFence GPUSubmitFrameTransferCommandBuffers();
GPUFence GPUSubmitFrameComputeCommandBuffers();

s64 GPUSwapchainImageCount();

struct GPUFrameStagingBuffer
{
	void *map;
	s64 size;
	VkBuffer vkBuffer;
	VkBuffer vkDestinationBuffer;

	void Flush();
	void FlushIn(GPUCommandBuffer cb);
	void *Map();
};

GPUFrameStagingBuffer NewGPUFrameStagingBuffer(s64 size, GPUBuffer dst);

struct GPUAsyncStagingBuffer
{
	VulkanMemoryAllocation *memory;
	VkBuffer vkBuffer;
	VkBuffer vkDestinationBuffer;

	void Flush();
	void FlushIn(GPUCommandBuffer cb);
	void *Map();
};

GPUAsyncStagingBuffer NewGPUAsyncStagingBuffer(s64 size, GPUBuffer dst);

void ClearGPUFrameResources(s64 frameIndex);

struct GPUGlobalUniforms
{
	u32 dummy;
};

struct GPUViewUniforms
{
	u32 dummy;
};

struct GPUMaterialUniforms
{
	u32 dummy;
};

struct GPUObjectUniforms
{
	M4 modelViewProjection;
};

struct GPUUniform
{
	s64 set;
	s64 index;
};

struct GPUUniformBufferWriteDescription
{
	GPUUniform uniform;
	s64 size; // @TODO: Get rid of this field. (The update function should know the size already based on the set.)
	void *data;
};

struct GPUUniformImageWriteDescription
{
	GPUUniform uniform;
	GPUSampler sampler;
	GPUImageView imageView;
	GPUImageLayout layout;
};

// @TODO: Copy uniform buffer.

GPUUniform GPUAddUniform(s64 set);
void GPUUpdateUniforms(ArrayView<GPUUniformBufferWriteDescription> us, ArrayView<GPUUniformImageWriteDescription> ts);

GPUFramebuffer NewGPUFramebuffer(s64 w, s64 h, ArrayView<GPUImageView> attachments);

VkRenderPass TEMPORARY_CREATE_RENDER_PASS();

#endif
