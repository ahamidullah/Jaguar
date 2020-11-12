#pragma once

#ifdef VulkanBuild

#include "Memory.h"
#include "Basic/Container/Map.h"
#include "Basic/Thread.h"

namespace GPU::Vulkan
{

struct Buffer
{
	VkBuffer buffer;
	s64 size;
	s64 offset;
	void *map;

	void Free();
};

struct BufferBlockList
{
	MemoryAllocator memoryAllocator;
	VkBufferUsageFlags bufferUsage;
	map::Map<VkDeviceMemory, VkBuffer> buffers;
};

struct BufferAllocator
{
	Spinlock lock;
	array::Array<BufferBlockList> blockLists;

	Buffer Allocate(PhysicalDevice pd, Device d, VkBufferUsageFlags bu, VkMemoryPropertyFlags mp, s64 size);
};

BufferAllocator NewBufferAllocator(PhysicalDevice pd, Device d);

}

#endif
