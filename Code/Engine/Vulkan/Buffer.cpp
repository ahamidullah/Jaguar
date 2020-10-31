#ifdef VulkanBuild

#include "Memory.h"

namespace GPU
{

struct BufferBlockList
{
	VkBufferUsageFlags bufferUsage;
	MemoryAllocator memoryAllocator;
	HashTable<VkDeviceMemory, VkBuffer> vkBuffers;
};

struct BufferAllocator
{
	Spinlock lock;
	Array<BufferBlockList> blockLists;

	Buffer Allocate(VkBufferUsageFlags bu, VkMemoryPropertyFlags mp, s64 size);
};

u64 HashVkDeviceMemory(VkDeviceMemory m)
{
	return HashPointer(m);
}

Buffer BufferAllocator::Allocate(VkBufferUsageFlags bu, VkMemoryPropertyFlags mp, s64 size)
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
		this->blockLists.Append(
			{
				.bufferUsage = bu,
				.vkBuffers = NewHashTableIn<VkDeviceMemory, VkBuffer>(GlobalAllocator(), 32, HashVkDeviceMemory),
			});
		list = this->blockLists.Last();
	}
	auto af = VkMemoryAllocateFlags{};
	if (bu & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
	{
		af |= VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
	}
	auto fail = false;
	auto mem = list->memoryAllocator.Allocate(mp, af, size, 1, &fail);
	Assert(!fail); // @TODO
	auto buf = VkBuffer{};
	if (auto b = list->vkBuffers.Lookup(mem.vkMemory); b)
	{
		buf = *b;
	}
	else
	{
		auto ci = VkBufferCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = (VkDeviceSize)mem.blockSize,
			.usage = list->bufferUsage,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		};
		VkCheck(vkCreateBuffer(vkDevice, &ci, NULL, &buf));
		VkCheck(vkBindBufferMemory(vkDevice, buf, mem.vkMemory, 0));
		list->vkBuffers.Insert(mem.vkMemory, buf);
	}
	ConsolePrint("MEM oFFSET: %ld.\n", mem.offset);
	return
	{
		.vkBuffer = buf,
		.size = size,
		.offset = mem.offset,
		.map = mem.map,
	};
}

auto bufferAllocator = []() -> BufferAllocator
{
	return
	{
		.blockLists = NewArrayIn<BufferBlockList>(GlobalAllocator(), 12),
	};
}();

Buffer NewBuffer(VkBufferUsageFlags bu, VkMemoryPropertyFlags mp, s64 size)
{
	return bufferAllocator.Allocate(bu, mp, size);
}

void Buffer::Free()
{
}

}

#endif
