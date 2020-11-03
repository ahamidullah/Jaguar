#pragma once

#ifdef VulkanBuild

#include "Buffer.h"
#include "CommandBuffer.h"

namespace GPU
{

struct StagingBuffer
{
	CommandBuffer commandBuffer;
	Buffer source;
	Buffer destination;
	s64 offset;
	void *map;

	void MapBuffer(Buffer dst, s64 offset);
	void Flush();

	private:
	void Upload();
};

}

#endif
