#pragma once

#ifdef VulkanBuild

#include "Shader.h"
#include "Framebuffer.h"
#include "Queue.h"

struct GPURenderBatch;

namespace GPU::Vulkan
{

struct Device;
struct PhysicalDevice;
struct Buffer;

struct CommandBuffer
{
	VkCommandBuffer commandBuffer;

	void BeginRenderPass(Shader s, Framebuffer fb);
	void EndRenderPass();
	void SetViewport(s64 w, s64 h);
	void SetScissor(s64 w, s64 h);
	void CopyBuffer(Buffer src, Buffer dst, s64 srcOffset, s64 dstOffset);
	void DrawRenderBatch(GPURenderBatch rb);
};

struct CommandBufferPool
{
	array::Static<array::Static<array::Array<VkCommandPool>, s64(QueueType::Count)>, 2 + 1> commandPools;
	array::Static<array::Static<array::Array<array::Array<VkCommandBuffer>>, s64(QueueType::Count)>, 2 + 1> recyclePools;
	array::Static<array::Array<VkCommandBuffer>, s64(QueueType::Count)> active;

	CommandBuffer Get(Device d, QueueType t);
	void Release(QueueType t, s64 threadIndex, array::View<VkCommandBuffer> cbs);
	void ClearActive(QueueType t);
	void ResetCommandPools(Device d, s64 frameIndex);
};

CommandBufferPool NewCommandBufferPool(PhysicalDevice pd, Device d);

}

#endif
