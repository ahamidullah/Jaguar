#ifdef VulkanBuild

namespace GPU::Vulkan
{

u32 MemoryHeapIndex(VkPhysicalDeviceMemoryProperties mp, VkMemoryPropertyFlags f)
{
	for (auto i = 0; i < mp.memoryTypeCount; i += 1)
	{
		if ((mp.memoryTypes[i].propertyFlags & f) == f)
		{
			return i;
		}
	}
	Abort("Vulkan", "Invalid memory property flags: %u.", f);
	return 0;
}

const auto DefaultBlockSize = 64 * Megabyte;

// @TODO: Take in a priority for GPU memory and allocate accordingly.
MemoryAllocation MemoryAllocator::Allocate(PhysicalDevice pd, Device d, VkMemoryPropertyFlags pf, VkMemoryAllocateFlags af, s64 size, s64 align, bool *fail)
{
	if (this->blockLists.count == 0)
	{
		this->blockLists.SetAllocator(Memory::GlobalHeap());
	}
	auto list = (MemoryBlockList *){};
	for (auto i = 0; i < this->blockLists.count; i += 1)
	{
		if (this->blockLists[i].propertyFlags == pf && this->blockLists[i].allocateFlags == af)
		{
			list = &this->blockLists[i];
			break;
		}
	}
	if (!list)
	{
		this->blockLists.Append(
			{
				.propertyFlags = pf,
				.allocateFlags = af,
				.heapIndex = MemoryHeapIndex(pd.memoryProperties, pf),
				.blockSize = DefaultBlockSize,
			});
		list = this->blockLists.Last();
	}
	// @TODO: Pool different allocation size categories together. Shouldn't be a single blockSize.
	// @TODO: Use free'd allocations.
	Assert(size < list->blockSize);
	if (list->blocks.count == 0 || Memory::AlignAddress(list->blocks.Last()->frontier, align) + size > list->blockSize)
	{
		auto fi = VkMemoryAllocateFlagsInfo
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
		};
		auto ai = VkMemoryAllocateInfo
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = VkDeviceSize(list->blockSize),
			.memoryTypeIndex = list->heapIndex,
		};
		if (af)
		{
			fi.flags = af;
			fi.deviceMask = 1;
			ai.pNext = &fi;
		}
		auto newBlk = MemoryBlock{};
		auto rc = vkAllocateMemory(d.device, &ai, NULL, &newBlk.memory);
		if (rc == VK_ERROR_OUT_OF_DEVICE_MEMORY || rc == VK_ERROR_OUT_OF_HOST_MEMORY)
		{
			Abort("Vulkan", "Failed to allocate memory for block allocator."); // @TODO
		}
		VkCheck(rc);
		newBlk.frontier = 0;
		if (list->propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
		{
			VkCheck(vkMapMemory(d.device, newBlk.memory, 0, list->blockSize, 0, &newBlk.map));
		}
		list->blocks.Append(newBlk);
	}
	list->blocks.Last()->frontier = Memory::AlignAddress(list->blocks.Last()->frontier, align);
	auto blk = list->blocks.Last();
	auto alloc = MemoryAllocation
	{
		.memory = blk->memory,
		.blockSize = list->blockSize,
		.offset = blk->frontier,
	};
	if (pf & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
	{
		alloc.map = (u8 *)(blk->map) + alloc.offset;
	}
	blk->frontier += size;
	Assert(blk->frontier <= list->blockSize);
	return alloc;
}

}

#endif
