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
	bool mapped;

	void MapBuffer(Buffer dst, s64 offset);
	void *Map();
	void Transfer();
	void Flush();
};

}

#endif
