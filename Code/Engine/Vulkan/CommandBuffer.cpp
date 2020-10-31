#ifdef VulkanBuild

#include "CommandBuffer.h"

namespace GPU
{

CommandBuffer NewCommandBuffer(QueueType t)
{
	auto cb = GPUFrameGraphicsCommandBuffer{};
	auto qt = 0;
	switch (t)
	{
	case QueueType::Graphics:
	{
		cb.vkCommandBuffer = NewVulkanFrameCommandBuffer(VulkanGraphicsQueue);
		qt = VulkanGraphicsQueue;
	} break;
	case QueueType::Transfer:
	{
		cb.vkCommandBuffer = NewVulkanFrameCommandBuffer(VulkanTransferQueue);
		qt = VulkanTransferQueue;
	} break;
	case QueueType::Compute:
	{
		cb.vkCommandBuffer = NewVulkanFrameCommandBuffer(VulkanComputeQueue);
		qt = VulkanComputeQueue;
	} break;
	case QueueType::Present:
	{
		cb.vkCommandBuffer = NewVulkanFrameCommandBuffer(VulkanPresentQueue);
		qt = VulkanPresentQueue;
	} break;
	case QueueType::Count:
	default:
	{
		Abort("Vulkan", "ERROR");
	} break;
	}
	return {cb.vkCommandBuffer, qt};
}

void CommandBuffer::BeginRenderPass(Shader s, Framebuffer fb)
{
	Assert(s.vkRenderPass);
	Assert(s.vkPipeline);
	auto vkFB = NewVkFramebuffer(s.vkRenderPass, fb);
	vkCmdBindPipeline(this->vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, s.vkPipeline);
	auto clearColor = VkClearValue
	{
		.color.float32 = {0.04f, 0.19f, 0.34f, 1.0f},
	};
	auto clearDepth = VkClearValue
	{
		.depthStencil.depth = 1.0f,
		.depthStencil.stencil = 0,
	};
	auto cvs = MakeStaticArray<VkClearValue>(clearColor, clearDepth);
	auto bi = VkRenderPassBeginInfo
	{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = s.vkRenderPass,
		.framebuffer = vkFB,
		.renderArea =
		{
			.offset = {0, 0},
			.extent = {(u32)RenderWidth(), (u32)RenderHeight()},
		},
		.clearValueCount = (u32)cvs.Count(),
		.pClearValues = cvs.elements,
	};
	vkCmdBeginRenderPass(this->vkCommandBuffer, &bi, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::EndRenderPass()
{
	vkCmdEndRenderPass(this->vkCommandBuffer);
}

void CommandBuffer::SetViewport(s64 w, s64 h)
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

void CommandBuffer::SetScissor(s64 w, s64 h)
{
	auto scissor = VkRect2D
	{
		.extent = {(u32)w, (u32)h},
	};
	vkCmdSetScissor(this->vkCommandBuffer, 0, 1, &scissor);
}

void CommandBuffer::CopyBuffer(Buffer src, Buffer dst, s64 srcOffset, s64 dstOffset)
{
	auto c = VkBufferCopy
	{
		.srcOffset = (VkDeviceSize)src.offset + srcOffset,
		.dstOffset = (VkDeviceSize)dst.offset + dstOffset,
		.size = (VkDeviceSize)src.size,
	};
	vkCmdCopyBuffer(this->vkCommandBuffer, src.vkBuffer, dst.vkBuffer, 1, &c);
}

void CommandBuffer::DrawRenderBatch(GPURenderBatch rb)
{
	vkCmdPushConstants(this->vkCommandBuffer, rb.vkPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VkDeviceAddress), &rb.drawBufferPointer);
	vkCmdBindIndexBuffer(this->vkCommandBuffer, rb.vkIndexBuffer, 0, GPUIndexTypeUint16);
	vkCmdDrawIndexedIndirect(this->vkCommandBuffer, rb.indirectCommands.vkBuffer, rb.indirectCommands.offset, rb.indirectCommandCount, sizeof(VkDrawIndexedIndirectCommand));
}

void CommandBuffer::Queue()
{
	VkCheck(vkEndCommandBuffer(this->vkCommandBuffer));
	vkFrameQueuedCommandBuffers[vkCommandGroupUseIndex][this->queueType][ThreadIndex()].Append(this->vkCommandBuffer);
}

}

#endif
