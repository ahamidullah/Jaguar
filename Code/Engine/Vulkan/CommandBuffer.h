#pragma once

#ifdef VulkanBuild

#include "Shader.h"
#include "Framebuffer.h"
#include "Queue.h"

namespace GPU
{

struct CommandBuffer
{
	VkCommandBuffer vkCommandBuffer;
	s64 queueType;

	void BeginRenderPass(Shader s, Framebuffer fb);
	void EndRenderPass();
	void SetViewport(s64 w, s64 h);
	void SetScissor(s64 w, s64 h);
	void CopyBuffer(Buffer src, Buffer dst, s64 srcOffset, s64 dstOffset);
	void DrawRenderBatch(GPURenderBatch rb);
	void Queue();
};

CommandBuffer NewCommandBuffer(QueueType t);

}

#endif
