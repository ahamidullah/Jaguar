#ifdef VulkanBuild

#include "Buffer.h"
#include "Memory.h"
#include "Vulkan.h"

namespace GPU::Vulkan
{

void Buffer::Free()
{
}

BufferAllocator NewBufferAllocator(PhysicalDevice pd, Device d)
{
	return
	{
		.blockLists = array::NewIn<BufferBlockList>(Memory::GlobalHeap(), 12),
	};
}

Buffer BufferAllocator::Allocate(PhysicalDevice pd, Device d, VkBufferUsageFlags bu, VkMemoryPropertyFlags mp, s64 size)
{
	this->lock.Lock();
	Defer(this->lock.Unlock());
	auto list = (BufferBlockList *){};
	for (auto i = 0; i < this->blockLists.count; i += 1)
	{
		if (this->blockLists[i].bufferUsage == bu)
		{
			list = &this->blockLists[i];
			break;
		}
	}
	if (!list)
	{
		auto HashVkDeviceMemory = [](VkDeviceMemory m) -> u64
		{
			return HashPointer(m);
		};
		this->blockLists.Append(
			{
				.bufferUsage = bu,
				.buffers = map::New<VkDeviceMemory, VkBuffer>(32, HashVkDeviceMemory),
			});
		list = this->blockLists.Last();
	}
	auto af = VkMemoryAllocateFlags{};
	if (bu & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
	{
		af |= VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
	}
	auto fail = false;
	auto mem = list->memoryAllocator.Allocate(pd, d, mp, af, size, 1, &fail);
	Assert(!fail); // @TODO
	auto buf = VkBuffer{};
	if (auto b = list->buffers.Lookup(mem.memory); b)
	{
		buf = *b;
	}
	else
	{
		auto ci = VkBufferCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = VkDeviceSize(mem.blockSize),
			.usage = list->bufferUsage,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		};
		Check(vkCreateBuffer(d.device, &ci, NULL, &buf));
		Check(vkBindBufferMemory(d.device, buf, mem.memory, 0));
		list->buffers.Insert(mem.memory, buf);
	}
	return
	{
		.buffer = buf,
		.size = size,
		.offset = mem.offset,
		.map = mem.map,
	};
}

}

#endif
