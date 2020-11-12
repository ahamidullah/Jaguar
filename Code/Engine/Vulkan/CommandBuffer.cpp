#ifdef VulkanBuild

#include "CommandBuffer.h"

namespace GPU::Vulkan
{

void CommandBuffer::BeginRenderPass(Shader s, Framebuffer fb)
{
	Assert(s.vkRenderPass);
	Assert(s.vkPipeline);
	auto vkFB = NewVkFramebuffer(s.vkRenderPass, fb);
	vkCmdBindPipeline(this->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, s.vkPipeline);
	auto clearColor = VkClearValue
	{
		.color.float32 = {0.04f, 0.19f, 0.34f, 1.0f},
	};
	auto clearDepth = VkClearValue
	{
		.depthStencil.depth = 1.0f,
		.depthStencil.stencil = 0,
	};
	auto cvs = array::MakeStatic<VkClearValue>(clearColor, clearDepth);
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
	vkCmdBeginRenderPass(this->commandBuffer, &bi, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::EndRenderPass()
{
	vkCmdEndRenderPass(this->commandBuffer);
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
	vkCmdSetViewport(this->commandBuffer, 0, 1, &v);
}

void CommandBuffer::SetScissor(s64 w, s64 h)
{
	auto scissor = VkRect2D
	{
		.extent = {(u32)w, (u32)h},
	};
	vkCmdSetScissor(this->commandBuffer, 0, 1, &scissor);
}

void CommandBuffer::CopyBuffer(Buffer src, Buffer dst, s64 srcOffset, s64 dstOffset)
{
	auto c = VkBufferCopy
	{
		.srcOffset = (VkDeviceSize)src.offset + srcOffset,
		.dstOffset = (VkDeviceSize)dst.offset + dstOffset,
		.size = (VkDeviceSize)src.size,
	};
	vkCmdCopyBuffer(this->commandBuffer, src.buffer, dst.buffer, 1, &c);
}

void CommandBuffer::DrawRenderBatch(GPURenderBatch rb)
{
	vkCmdPushConstants(this->commandBuffer, rb.vkPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VkDeviceAddress), &rb.drawBufferPointer);
	vkCmdBindIndexBuffer(this->commandBuffer, rb.vkIndexBuffer, 0, VK_INDEX_TYPE_UINT16);
	vkCmdDrawIndexedIndirect(this->commandBuffer, rb.indirectCommands.buffer, rb.indirectCommands.offset, rb.indirectCommandCount, sizeof(VkDrawIndexedIndirectCommand));
}

CommandBufferPool NewCommandBufferPool(PhysicalDevice pd, Device d)
{
	auto p = CommandBufferPool{};
	auto ci = VkCommandPoolCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
	};
	for (auto i = 0; i < MaxFramesInFlight + 1; i += 1)
	{
		for (auto j = 0; j < s64(QueueType::Count); j += 1)
		{
			p.commandPools[i][j] = array::NewIn<VkCommandPool>(Memory::GlobalHeap(), WorkerThreadCount());
			for (auto k = 0; k < WorkerThreadCount(); k += 1)
			{
				ci.queueFamilyIndex = pd.queueFamilies[j];
				Check(vkCreateCommandPool(d.device, &ci, NULL, &p.commandPools[i][j][k]));
			}
		}
	}
	for (auto i = 0; i < s64(QueueType::Count); i += 1)
	{
		p.active[i] = array::NewIn<VkCommandBuffer>(Memory::GlobalHeap(), WorkerThreadCount());
	}
	for (auto i = 0; i < MaxFramesInFlight + 1; i += 1)
	{
		for (auto j = 0; j < s64(QueueType::Count); j += 1)
		{
			p.recyclePools[i][j] = array::NewIn<array::Array<VkCommandBuffer>>(Memory::GlobalHeap(), WorkerThreadCount());
			for (auto &rp : p.recyclePools[i][j])
			{
				rp = array::NewIn<VkCommandBuffer>(Memory::GlobalHeap(), 0);
			}
		}
	}
	return p;
}

CommandBuffer CommandBufferPool::Get(Device d, QueueType t)
{
	auto cb = CommandBuffer{};
	if (this->active[s64(t)][ThreadIndex()])
	{
		cb.commandBuffer = this->active[s64(t)][ThreadIndex()];
		return cb;
	}
	auto bi = VkCommandBufferBeginInfo
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};
	if (this->recyclePools[vkCommandGroupUseIndex][s64(t)][ThreadIndex()].count > 0)
	{
		cb.commandBuffer = this->recyclePools[vkCommandGroupUseIndex][s64(t)][ThreadIndex()].Pop();
		vkBeginCommandBuffer(cb.commandBuffer, &bi);
		this->active[s64(t)][ThreadIndex()] = cb.commandBuffer;
		return cb;
	}
    auto ai = VkCommandBufferAllocateInfo
    {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = this->commandPools[vkCommandGroupUseIndex][s64(t)][ThreadIndex()],
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
    };
    vkAllocateCommandBuffers(d.device, &ai, &cb.commandBuffer);
	vkBeginCommandBuffer(cb.commandBuffer, &bi);
	this->active[s64(t)][ThreadIndex()] = cb.commandBuffer;
	return cb;
}

void CommandBufferPool::Release(QueueType t, s64 threadIndex, array::View<VkCommandBuffer> cbs)
{
	this->recyclePools[vkCommandGroupFreeIndex][s64(t)][threadIndex].AppendAll(cbs);
}

void CommandBufferPool::ClearActive(QueueType t)
{
	for (auto &cb : this->active[s64(t)])
	{
		cb = NULL;
	}
}

void CommandBufferPool::ResetCommandPools(Device d, s64 frameIndex)
{
	for (auto tl : this->commandPools[vkCommandGroupFreeIndex])
	{
		for (auto p : tl)
		{
			Check(vkResetCommandPool(d.device, p, 0));
		}
	}
}

}

#endif
