#pragma once

#ifdef VulkanBuild

#include "Buffer.h"
#include "CommandBuffer.h"

namespace GPU::Vulkan
{

struct StagingBuffer
{
	PhysicalDevice *physicalDevice;
	Device *device;
	CommandBufferPool *commandBufferPool;
	CommandBuffer commandBuffer;
	BufferAllocator *bufferAllocator;
	Buffer source;
	Buffer destination;
	s64 offset;
	void *map;

	void MapBuffer(Buffer dst, s64 offset);
	void Upload();
	void Flush();
};

StagingBuffer NewStagingBuffer(PhysicalDevice *pd, Device *d, CommandBufferPool *p, BufferAllocator *b);

}

#endif
