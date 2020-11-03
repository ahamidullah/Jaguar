#pragma once

#ifdef VulkanBuild

namespace GPU
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
	Array<MemoryBlock> blocks;
};

struct MemoryAllocator
{
	Array<MemoryBlockList> blockLists;

	MemoryAllocation Allocate(VkMemoryPropertyFlags mp, VkMemoryAllocateFlags ma, s64 size, s64 align, bool *fail);
};

void InitializeMemory();

}

#endif
