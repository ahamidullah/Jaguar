#ifdef VulkanBuild

namespace GPU
{

// From https://zeux.io/2020/02/27/writing-an-efficient-vulkan-renderer/:
//
// VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT – this is generally referring to GPU memory that is not directly visible from CPU; it’s fastest
// to access from the GPU and this is the memory you should be using to store all render targets, GPU-only resources such as buffers for
// compute, and also all static resources such as textures and geometry buffers.
//
// VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT – on AMD hardware, this memory type refers to up to 256 MB
// of video memory that the CPU can write to directly, and is perfect for allocating reasonable amounts of data that is written by CPU
// every frame, such as uniform buffers or dynamic vertex/index buffers
//
// VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT2 – this is referring to CPU memory that is directly
// visible from GPU; reads from this memory go over PCI-express bus. In absence of the previous memory type, this generally speaking
// should be the choice for uniform buffers or dynamic vertex/index buffers, and also should be used to store staging buffers that are
// used to populate static resources allocated with VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT with data.
// 
// VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT – this is referring to GPU memory that might never need
// to be allocated for render targets on tiled architectures. It is recommended to use lazily allocated memory to save physical memory
// for large render targets that are never stored to, such as MSAA images or depth images. On integrated GPUs, there is no distinction
// between GPU and CPU memory – these devices generally expose VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
// that you can allocate all static resources through as well.
//
// When dealing with dynamic resources, in general allocating in non-device-local host-visible memory works well – it simplifies the
// application management and is efficient due to GPU-side caching of read-only data. For resources that have a high degree of random
// access though, like dynamic textures, it’s better to allocate them in VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT and upload data using
// staging buffers allocated in VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT memory – similarly to how you would handle static textures. In some
// cases you might need to do this for buffers as well – while uniform buffers typically don’t suffer from this, in some applications
// using large storage buffers with highly random access patterns will generate too many PCIe transactions unless you copy the buffers
// to GPU first; additionally, host memory does have higher access latency from the GPU side that can impact performance for many small
// draw calls.
//
// When allocating resources from VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, in case of VRAM oversubscription you can run out of memory; in
// this case you should fall back to allocating the resources in non-device-local VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT memory. Naturally
// you should make sure that large frequently used resources such as render targets are allocated first. There are other things you can
// do in an event of an oversubscription, such as migrating resources from GPU memory to CPU memory for less frequently used resources –
// this is outside of the scope of this article; additionally, on some operating systems like Windows 10 correct handling of
// oversubscription requires APIs that are not currently available in Vulkan.

u32 MemoryHeapIndex(VkMemoryPropertyFlags f)
{
	auto mp = &physicalDevice.memoryProperties;
	for (auto i = 0; i < mp->memoryTypeCount; i += 1)
	{
		if ((mp->memoryTypes[i].propertyFlags & f) == f)
		{
			return i;
		}
	}
	Abort("Vulkan", "Invalid memory property flags: %u.", f);
	return 0;
}

const auto DefaultBlockSize = 64 * Megabyte;

// @TODO: Take in a priority for GPU memory and allocate accordingly.
MemoryAllocation MemoryAllocator::Allocate(VkMemoryPropertyFlags pf, VkMemoryAllocateFlags af, s64 size, s64 align, bool *fail)
{
	if (this->blockLists.count == 0)
	{
		this->blockLists.SetAllocator(GlobalAllocator());
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
				.heapIndex = MemoryHeapIndex(pf),
				.blockSize = DefaultBlockSize,
			});
		list = this->blockLists.Last();
	}
	// @TODO: Pool different allocation size categories together. Shouldn't be a single blockSize.
	// @TODO: Use free'd allocations.
	Assert(size < list->blockSize);
	if (list->blocks.count == 0 || AlignAddress(list->blocks.Last()->frontier, align) + size > list->blockSize)
	{
		auto fi = VkMemoryAllocateFlagsInfo
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
		};
		auto ai = VkMemoryAllocateInfo
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = (VkDeviceSize)list->blockSize,
			.memoryTypeIndex = list->heapIndex,
		};
		if (af)
		{
			fi.flags = af;
			fi.deviceMask = 1;
			ai.pNext = &fi;
		}
		auto newBlk = MemoryBlock{};
		auto rc = vkAllocateMemory(vkDevice, &ai, NULL, &newBlk.vkMemory);
		if (rc == VK_ERROR_OUT_OF_DEVICE_MEMORY || rc == VK_ERROR_OUT_OF_HOST_MEMORY)
		{
			Abort("Vulkan", "Failed to allocate memory for block allocator."); // @TODO
		}
		VkCheck(rc);
		newBlk.frontier = 0;
		if (list->propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
		{
			VkCheck(vkMapMemory(vkDevice, newBlk.vkMemory, 0, list->blockSize, 0, &newBlk.map));
		}
		list->blocks.Append(newBlk);
	}
	list->blocks.Last()->frontier = AlignAddress(list->blocks.Last()->frontier, align);
	auto blk = list->blocks.Last();
	auto alloc = MemoryAllocation
	{
		.vkMemory = blk->vkMemory,
		.blockSize = list->blockSize,
		.offset = blk->frontier,
	};
	if (pf & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
	{
		alloc.map = ((u8 *)blk->map) + alloc.offset;
	}
	blk->frontier += size;
	Assert(blk->frontier <= list->blockSize);
	return alloc;
}

}

#endif
