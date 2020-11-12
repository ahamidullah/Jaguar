#pragma once

#ifdef VulkanBuild

namespace GPU::Vulkan
{

const auto MaxFramesInFlight = 2;

struct MemoryAllocation
{
	VkDeviceMemory memory;
	s64 blockSize;
	s64 offset;
	void *map;
};

struct MemoryBlock
{
	VkDeviceMemory memory;
	s64 frontier;
	void *map;
};

struct MemoryBlockList
{
	VkMemoryPropertyFlags propertyFlags;
	VkMemoryAllocateFlags allocateFlags;
	u32 heapIndex;
	s64 blockSize;
	array::Array<MemoryBlock> blocks;
};

struct MemoryAllocator
{
	array::Array<MemoryBlockList> blockLists;

	MemoryAllocation Allocate(PhysicalDevice pd, Device d, VkMemoryPropertyFlags mp, VkMemoryAllocateFlags ma, s64 size, s64 align, bool *fail);
};

}

#endif
