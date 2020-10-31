#pragma once

#ifdef VulkanBuild

namespace GPU
{

struct Buffer
{
	VkBuffer vkBuffer;
	s64 size;
	s64 offset;
	void *map;

	void Free();
};

Buffer NewBuffer(VkBufferUsageFlags bu, VkMemoryPropertyFlags mp, s64 size);

}

#endif
