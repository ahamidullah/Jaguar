#ifdef VulkanBuild

#include "StagingBuffer.h"

namespace GPU
{

#include "CommandBuffer.h"

void StagingBuffer::MapBuffer(Buffer dst, s64 offset)
{
	if (this->map)
	{
		this->Upload();
	}
	if (!this->commandBuffer.vkCommandBuffer)
	{
		this->commandBuffer = NewCommandBuffer(QueueType::Transfer);
	}
	this->source.Free();
	this->offset = offset;
	this->destination = dst;
	if (dst.map)
	{
		// Destination buffer is allocated from CPU accessible memory which is already mapped, so the staging buffer will write directly to
		// mapped memory, no source buffer needed.
		this->map = (u8 *)this->destination.map + offset;
		return;
	}
	this->source = NewBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, dst.size);
	this->map = this->source.map;
}

void StagingBuffer::Upload()
{
	this->map = NULL;
	if (this->destination.map)
	{
		return;
	}
	this->commandBuffer.CopyBuffer(this->source, this->destination, 0, this->offset);
}

void StagingBuffer::Flush()
{
	if (this->map)
	{
		this->Upload();
	}
	this->commandBuffer.Queue();
}

}

#endif
