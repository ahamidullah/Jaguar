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

struct GPUShader
{
	Array<VkShaderStageFlagBits> vkStages;
	Array<VkShaderModule> vkModules;
	VkRenderPass vkRenderPass;
	VkPipeline vkPipeline;
};

//GPUShader CompileGPUShader(String filename, bool *err);

struct GPUMemoryHeapInfo
{
	u64 usage;
	u64 budget;
};

GPUMemoryHeapInfo GPUMemoryUsage();
void LogGPUMemoryInfo();

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

struct VulkanSubbuffer
{
	VkBuffer vkBuffer;
	s64 offset;
};

typedef VulkanSubbuffer GPUBuffer;

GPUBuffer NewGPUVertexBuffer(s64 size);
GPUBuffer NewGPUIndexBuffer(s64 size);
GPUBuffer NewGPUFrameIndirectBuffer(s64 size);

struct GPUImage
{
	VkImage vkImage;

	void Free();
};

GPUImage NewGPUImage(s64 w, s64 h, GPUFormat f, GPUImageLayout il, GPUImageUsageFlags uf, GPUSampleCount sc);

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

struct GPUImageView
{
	VkImageView vkImageView;
};

GPUImageView NewGPUImageView(GPUImage src, GPUImageViewType t, GPUFormat f, GPUSwizzleMapping sm, GPUImageSubresourceRange isr);

struct GPUSampler
{
	VkSampler vkSampler;
};

struct GPUFramebuffer
{
	u64 id;
	u32 width;
	u32 height;
	Array<VkImageView> attachments;
};

GPUFramebuffer GPUDefaultFramebuffer();

/*
// A uniform may have more than one instance if it is updated while in use.
struct VulkanUniformInstance
{
	bool inUse;
	VkBuffer buffer;
};

struct VulkanUniform
{
	s64 size;
	s64 index;
	Array<VulkanUniformInstance> instances;
};

VulkanUniform NewVulkanUniform();

struct GPUUniformBufferWriteDescription
{
	GPUUniform *uniform;
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

void UpdateGPUUniforms(ArrayView<GPUUniformBufferWriteDescription> us, ArrayView<GPUUniformImageWriteDescription> ts);
*/

struct GPUSubmesh
{
	//u32 vertexOffset;
	//u32 indexOffset;
	u32 indexCount;
};

#include "Vulkan/Buffer.h"

struct GPUMeshAsset
{
	u32 firstIndex;
	s32 vertexOffset; // @TODO: Get rid of this field?
	Array<GPUSubmesh> submeshes;
	// @TODO: Get rid of the offsets?
	GPU::Buffer vertexBuffer;
	GPU::Buffer indexBuffer;
};

struct GPUMeshAssetCreateInfo
{
	s64 vertexCount;
	s64 vertexSize;
	s64 indexCount;
	s64 indexSize;
	ArrayView<u32> submeshIndices;
};

void NewGPUMeshAssetBlock(Memory::Allocator *a, ArrayView<GPUMeshAssetCreateInfo> cis, ArrayView<GPUMeshAsset *> out);
GPUMeshAsset NewGPUMeshAsset(Memory::Allocator *a, s64 vertCount, s64 vertSize, s64 indCount, s64 indSize, ArrayView<u32> submeshInds);

struct GPUUniform_
{
	GPU::Buffer buffer;
};

struct GPUMesh
{
	GPUMeshAsset *asset;
	GPU::Buffer uniform;
};

void NewGPUMeshBlock(ArrayView<GPUMeshAsset *> as, ArrayView<GPUMesh *> out);
GPUMesh NewGPUMesh(GPUMeshAsset *a);

struct GPUMaterial
{
	VkDescriptorSet vkDescriptorSet;
	s64 elementIndex;
	GPU::Buffer uniform;
};

Array<GPUMaterial> NewGPUMaterialBlock();
GPUMaterial NewGPUMaterial();

struct GPURenderPacket
{
	GPUMesh *mesh;
	GPUMaterial *material;
	// Array<GPUMaterial *> materials;
	//s64 bindingGroupIndex;
};

GPURenderPacket NewGPURenderPacket(GPUMesh *mesh, GPUMaterial *mat);

struct VulkanDrawCallBindingGroup
{
	VkBuffer vkVertexBuffer;
	VkBuffer vkIndexBuffer;
	ArrayView<VkDescriptorSet> vkObjectDescriptorSet;
	VkDescriptorSet vkMaterialDescriptorSet;

	bool operator==(VulkanDrawCallBindingGroup b);
};

struct VulkanDrawCall
{
	VkDescriptorSet vkObjectIndexDescriptorSet;
	VkDescriptorSet vkMaterialIndexDescriptorSet;
	VulkanSubbuffer indirectCommands;
	s64 indirectCommandCount;
};

struct GPURenderBatch
{
	//Array<VulkanDrawCall> drawCalls;
	VkDeviceAddress drawBufferPointer;
	GPU::Buffer indirectCommands;
	s64 indirectCommandCount;
	VkBuffer vkIndexBuffer;
	VkPipelineLayout vkPipelineLayout;
};

GPURenderBatch NewGPUFrameRenderBatch(ArrayView<GPURenderPacket> ps);

#if 0
struct GPUMeshAsset
{
	u32 indexCount;
	u32 instanceCount;
	u32 firstIndex;
	s32 vertexOffset;
	// @TODO: Get rid of the offsets?
	GPUBuffer vertexBuffer;
	GPUBuffer indexBuffer;

	GPUBuffer VertexBuffer();
	GPUBuffer IndexBuffer();
};

GPUMeshAsset NewGPUMeshAsset(s64 vertSize, s64 indSize);

struct GPUMesh
{
	u32 indexCount;
	u32 instanceCount;
	u32 firstIndex;
	s32 vertexOffset;
	s64 renderGroupIndex;
};

struct GPUModelAsset
{
	mesh
	materials
	uniform?
	well what goes here?
};

struct GPUModel
{
	Array<GPUMesh> meshes;
	Array<Array<GPUMaterial>> materials;
	GPUUniform objectUniform;
};

GPUMesh NewGPUMesh(GPUMeshAsset asset, ArrayView<GPUUniform> uniforms);

#endif

struct GPUCommandBuffer
{
	VkCommandBuffer vkCommandBuffer;

	void BeginRenderPass(GPUShader s, GPUFramebuffer fb);
	void EndRenderPass();
	void SetViewport(s64 w, s64 h);
	void SetScissor(s64 w, s64 h);
	void BindVertexBuffer(GPUBuffer b, s64 bindPoint);
	void BindIndexBuffer(GPUBuffer b, GPUIndexType t);
	void DrawIndexed(s64 numIndices, s64 firstIndex, s64 vertexOffset);
	void DrawIndexedIndirect(GPUBuffer cmdBuf, s64 count);
	void DrawRenderBatch(GPURenderBatch rb);
	void CopyBuffer(s64 size, GPUBuffer src, GPUBuffer dst, s64 srcOffset, s64 dstOffset);
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

struct GPUFrameStagingBuffer
{
	void *map;
	s64 size;
	GPUBuffer source;
	GPUBuffer destination;
	s64 offset;

	void Flush();
	void FlushIn(GPUCommandBuffer cb);
	void *Map();
};

GPUFrameStagingBuffer NewGPUFrameStagingBufferX(GPUBuffer dst, s64 size, s64 offset);

struct GPUAsyncStagingBuffer
{
	VkBuffer vkBuffer;
	VkBuffer vkDestinationBuffer;

	void Flush();
	void FlushIn(GPUCommandBuffer cb);
	void *Map();
};

//GPUAsyncStagingBuffer NewGPUAsyncStagingBuffer(s64 size, GPUBuffer dst);

GPUFramebuffer NewGPUFramebuffer(s64 w, s64 h, ArrayView<GPUImageView> attachments);

VkRenderPass TEMPORARY_CREATE_RENDER_PASS();

void InitializeGPU(Window *w);
void GPUBeginFrame();
void GPUEndFrame();
s64 GPUSwapchainImageCount();

#endif
