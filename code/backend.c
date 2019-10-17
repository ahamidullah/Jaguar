// @TODO: Maybe pull out device from the GPU_Context?

VkMemoryPropertyFlags GPU_Convert_To_Vulkan_Memory_Type(GPU_Memory_Type memory_type) {
	switch (memory_type) {
	case GPU_DEVICE_MEMORY: {
		return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	} break;
	case GPU_HOST_MEMORY: {
		return (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	} break;
	}
	Invalid_Code_Path();
	return 0;
}

VkBufferUsageFlags GPU_Convert_To_Vulkan_Buffer_Usage_Flags(GPU_Buffer_Usage_Flags flags) {
	VkBufferUsageFlags vulkan_flags = 0;
	if (flags & GPU_TRANSFER_DESTINATION_BUFFER) {
		vulkan_flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	}
	if (flags & GPU_TRANSFER_SOURCE_BUFFER) {
		vulkan_flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	}
	if (flags & GPU_VERTEX_BUFFER) {
		vulkan_flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	}
	if (flags & GPU_INDEX_BUFFER) {
		vulkan_flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	}
	if (flags & GPU_UNIFORM_BUFFER) {
		vulkan_flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	}
	return vulkan_flags;
}

VkFormat GPU_Convert_To_Vulkan_Format(GPU_Format format) {
	switch (format) {
	case GPU_FORMAT_R32G32B32_SFLOAT: {
		return VK_FORMAT_R32G32B32_SFLOAT;
	} break;
	case GPU_FORMAT_R32_UINT: {
		return VK_FORMAT_R32_UINT;
	} break;
	case GPU_FORMAT_D32_SFLOAT_S8_UINT: {
		return VK_FORMAT_D32_SFLOAT_S8_UINT;
	} break;
	case GPU_FORMAT_R8G8B8A8_UNORM: {
		return VK_FORMAT_R8G8B8A8_UNORM;
	} break;
	case GPU_FORMAT_D16_UNORM: {
		return VK_FORMAT_D16_UNORM;
	} break;
	case GPU_FORMAT_UNDEFINED: {
		return VK_FORMAT_UNDEFINED;
	} break;
	}
	Invalid_Code_Path();
	return 0;
}

VkImageLayout GPU_Convert_To_Vulkan_Image_Layout(GPU_Image_Layout layout) {
	switch (layout) {
	case GPU_IMAGE_LAYOUT_UNDEFINED: {
		return VK_IMAGE_LAYOUT_UNDEFINED;
	} break;
	case GPU_IMAGE_LAYOUT_GENERAL: {
		return VK_IMAGE_LAYOUT_GENERAL;
	} break;
	case GPU_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: {
		return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	} break;
	case GPU_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: {
		return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	} break;
	case GPU_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL: {
		return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	} break;
	case GPU_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: {
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	} break;
	case GPU_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: {
		return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL ;
	} break;
	case GPU_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: {
		return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	} break;
	case GPU_IMAGE_LAYOUT_PREINITIALIZED: {
		return VK_IMAGE_LAYOUT_PREINITIALIZED;
	} break;
	case GPU_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL: {
		return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
	} break;
	case GPU_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL: {
		return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
	} break;
	case GPU_IMAGE_LAYOUT_PRESENT_SRC_KHR: {
		return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	} break;
	case GPU_IMAGE_LAYOUT_SHARED_PRESENT_KHR: {
		return VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR;
	} break;
	case GPU_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV: {
		return VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV;
	} break;
	case GPU_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT: {
		return VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;
	} break;
	case GPU_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR: {
		return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR ;
	} break;
	case GPU_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR: {
		return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR;
	} break;
	}
	Invalid_Code_Path();
	return 0;
}

VkImageUsageFlags GPU_Convert_To_Vulkan_Image_Usage_Flags(GPU_Image_Usage_Flags usage_flags) {
	VkImageUsageFlags vulkan_usage_flags = 0;
	if (usage_flags & GPU_IMAGE_USAGE_TRANSFER_SRC) {
		vulkan_usage_flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}
	if (usage_flags & GPU_IMAGE_USAGE_TRANSFER_DST) {
		vulkan_usage_flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	if (usage_flags & GPU_IMAGE_USAGE_SAMPLED) {
		vulkan_usage_flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
	}
	if (usage_flags & GPU_IMAGE_USAGE_STORAGE) {
		vulkan_usage_flags |= VK_IMAGE_USAGE_STORAGE_BIT;
	}
	if (usage_flags & GPU_IMAGE_USAGE_COLOR_ATTACHMENT) {
		vulkan_usage_flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}
	if (usage_flags & GPU_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT) {
		vulkan_usage_flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}
	if (usage_flags & GPU_IMAGE_USAGE_TRANSIENT_ATTACHMENT) {
		vulkan_usage_flags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
	}
	if (usage_flags & GPU_IMAGE_USAGE_INPUT_ATTACHMENT) {
		vulkan_usage_flags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
	}
	if (usage_flags & GPU_IMAGE_USAGE_SHADING_RATE_IMAGE_NV) {
		vulkan_usage_flags |= VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV;
	}
	if (usage_flags & GPU_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_EXT) {
		vulkan_usage_flags |= VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
	}
	return vulkan_usage_flags;
}

VkSampleCountFlags GPU_Convert_To_Vulkan_Sample_Count(GPU_Sample_Count_Flags sample_count_flags) {
	VkSampleCountFlags vulkan_sample_count_flags = 0;
	if (sample_count_flags & GPU_SAMPLE_COUNT_1) {
		vulkan_sample_count_flags |= VK_SAMPLE_COUNT_1_BIT;
	}
	if (sample_count_flags & GPU_SAMPLE_COUNT_2) {
		vulkan_sample_count_flags |= VK_SAMPLE_COUNT_2_BIT;
	}
	if (sample_count_flags & GPU_SAMPLE_COUNT_4) {
		vulkan_sample_count_flags |= VK_SAMPLE_COUNT_4_BIT;
	}
	if (sample_count_flags & GPU_SAMPLE_COUNT_8) {
		vulkan_sample_count_flags |= VK_SAMPLE_COUNT_8_BIT;
	}
	if (sample_count_flags & GPU_SAMPLE_COUNT_16) {
		vulkan_sample_count_flags |= VK_SAMPLE_COUNT_16_BIT;
	}
	if (sample_count_flags & GPU_SAMPLE_COUNT_32) {
		vulkan_sample_count_flags |= VK_SAMPLE_COUNT_32_BIT;
	}
	if (sample_count_flags & GPU_SAMPLE_COUNT_64) {
		vulkan_sample_count_flags |= VK_SAMPLE_COUNT_64_BIT;
	}
	return vulkan_sample_count_flags;
}

VkImageCreateInfo GPU_Convert_To_Vulkan_Image_Create_Info(GPU_Image_Creation_Parameters *parameters) {
	return (VkImageCreateInfo){
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.extent.width = parameters->width,
		.extent.height = parameters->height,
		.extent.depth = 1,
		.mipLevels = 1,
		.arrayLayers = 1,
		.format = GPU_Convert_To_Vulkan_Format(parameters->format),
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.initialLayout = GPU_Convert_To_Vulkan_Image_Layout(parameters->initial_layout),
		.usage = GPU_Convert_To_Vulkan_Image_Usage_Flags(parameters->usage_flags),
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.samples = GPU_Convert_To_Vulkan_Sample_Count(parameters->sample_count_flags),
	};
}

bool GPU_Allocate_Memory(GPU_Context *context, u32 size, GPU_Memory_Type memory_type, GPU_Memory *memory) {
	switch (context->active_render_api) {
	case VULKAN_RENDER_API: {
		return Vulkan_Allocate_Memory(&context->vulkan, size, GPU_Convert_To_Vulkan_Memory_Type(memory_type), &memory->vulkan);
	} break;
	}
	Invalid_Code_Path();
	return 0;
}

void *GPU_Map_Memory(GPU_Context *context, GPU_Memory memory, u32 size, u32 offset) {
	switch (context->active_render_api) {
	case VULKAN_RENDER_API: {
		return Vulkan_Map_Memory(&context->vulkan, memory.vulkan, size, offset);
	} break;
	}
	Invalid_Code_Path();
	return NULL;
}

GPU_Buffer GPU_Create_Buffer(GPU_Context *context, GPU_Buffer_Usage_Flags usage_flags, u32 size) {
	switch (context->active_render_api) {
	case VULKAN_RENDER_API: {
		VkBufferCreateInfo buffer_create_info = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = size,
			.usage = GPU_Convert_To_Vulkan_Buffer_Usage_Flags(usage_flags),
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		};
		GPU_Buffer buffer;
		buffer.vulkan = Vulkan_Create_Buffer(&context->vulkan, &buffer_create_info);
		return buffer;
	} break;
	}
	Invalid_Code_Path();
	return (GPU_Buffer){};
}

GPU_Image GPU_Create_Image(GPU_Context *context, GPU_Image_Creation_Parameters *parameters, GPU_Image_Allocation image_allocation) {
	switch (context->active_render_api) {
	case VULKAN_RENDER_API: {
		GPU_Image image;
		VkImageCreateInfo image_create_info = GPU_Convert_To_Vulkan_Image_Create_Info(parameters);
		Vulkan_Create_Image(&context->vulkan, &image_create_info, image_allocation.memory.vulkan, *image_allocation.offset, &image.vulkan.image, &image.vulkan.view);
		return image;
	} break;
	}
	Invalid_Code_Path();
	return (GPU_Image){};
}

GPU_Resource_Allocation_Requirements GPU_Get_Buffer_Allocation_Requirements(GPU_Context *context, GPU_Buffer buffer) {
	switch (context->active_render_api) {
	case VULKAN_RENDER_API: {
		VkMemoryRequirements memory_requirements = Vulkan_Get_Buffer_Allocation_Requirements(&context->vulkan, buffer.vulkan);
		return (GPU_Resource_Allocation_Requirements){
			.size = memory_requirements.size,
			.alignment = memory_requirements.alignment,
		};
	} break;
	}
	Invalid_Code_Path();
	return (GPU_Resource_Allocation_Requirements){};
}

void GPU_Bind_Buffer_Memory(GPU_Context *context, GPU_Buffer buffer, GPU_Memory memory, u32 offset) {
	switch (context->active_render_api) {
	case VULKAN_RENDER_API: {
		return Vulkan_Bind_Buffer_Memory(&context->vulkan, buffer.vulkan, memory.vulkan, offset);
	} break;
	}
	Invalid_Code_Path();
}

GPU_Resource_Allocation_Requirements GPU_Get_Image_Allocation_Requirements(GPU_Context *context, GPU_Image_Creation_Parameters *parameters) {
	switch (context->active_render_api) {
	case VULKAN_RENDER_API: {
		VkImageCreateInfo image_create_info = GPU_Convert_To_Vulkan_Image_Create_Info(parameters);
		VkMemoryRequirements memory_requirements = Vulkan_Get_Image_Allocation_Requirements(&context->vulkan, &image_create_info);
		return (GPU_Resource_Allocation_Requirements){
			.size = memory_requirements.size,
			.alignment = memory_requirements.alignment,
		};
	} break;
	}
	Invalid_Code_Path();
	return (GPU_Resource_Allocation_Requirements){};
}

GPU_Swapchain GPU_Create_Swapchain(GPU_Context *context) {
	switch (context->active_render_api) {
	case VULKAN_RENDER_API: {
		GPU_Swapchain swapchain;
		swapchain.vulkan = Vulkan_Create_Swapchain(&context->vulkan);
		return swapchain;
	} break;
	}
	Invalid_Code_Path();
	return (GPU_Swapchain){};
}

u32 GPU_Get_Swapchain_Image_Count(GPU_Context *context, GPU_Swapchain swapchain) {
	switch (context->active_render_api) {
	case VULKAN_RENDER_API: {
		return Vulkan_Get_Swapchain_Image_Count(&context->vulkan, swapchain.vulkan);
	} break;
	}
	Invalid_Code_Path();
	return 0;
}

void GPU_Get_Swapchain_Images(GPU_Context *context, GPU_Swapchain swapchain, u32 count, GPU_Image *images) {
	switch (context->active_render_api) {
	case VULKAN_RENDER_API: {
		VkImage vulkan_images[count];
		VkImageView vulkan_image_views[count];
		Vulkan_Get_Swapchain_Images(&context->vulkan, swapchain.vulkan, count, vulkan_images, vulkan_image_views);
		for (s32 i = 0; i < count; i++) {
			images[i].vulkan.image = vulkan_images[i];
			images[i].vulkan.view = vulkan_image_views[i];
		}
		return;
	} break;
	}
	Invalid_Code_Path();
}

GPU_Fence GPU_Create_Fence(GPU_Context *context, bool start_signalled) {
	switch (context->active_render_api) {
	case VULKAN_RENDER_API: {
		GPU_Fence fence;
		fence.vulkan = Vulkan_Create_Fence(&context->vulkan, start_signalled);
		return fence;
	} break;
	}
	Invalid_Code_Path();
	return (GPU_Fence){};
}

GPU_Command_Buffer GPU_Create_Command_Buffer(GPU_Context *context, GPU_Command_Pool command_pool) {
	switch (context->active_render_api) {
	case VULKAN_RENDER_API: {
		GPU_Command_Buffer buffer;
		buffer.vulkan = Vulkan_Create_Command_Buffer(&context->vulkan, command_pool.vulkan);
		return buffer;
	} break;
	}
	Invalid_Code_Path();
	return (GPU_Command_Buffer){};
}

void GPU_Record_Copy_Buffer_Commands(GPU_Context *context, GPU_Command_Buffer command_buffer, u32 count, u32 *sizes, GPU_Buffer source, GPU_Buffer destination, u32 *source_offsets, u32 *destination_offsets) {
	switch (context->active_render_api) {
	case VULKAN_RENDER_API: {
		Vulkan_Record_Copy_Buffer_Commands(&context->vulkan, command_buffer.vulkan, count, sizes, source.vulkan, destination.vulkan, source_offsets, destination_offsets);
		return;
	} break;
	}
	Invalid_Code_Path();
}


void GPU_End_Command_Buffer(GPU_Context *context, GPU_Command_Buffer command_buffer) {
	switch (context->active_render_api) {
	case VULKAN_RENDER_API: {
		vkEndCommandBuffer(command_buffer.vulkan);
		return;
	} break;
	}
	Invalid_Code_Path();
}

void GPU_Initialize(GPU_Context *context) {
	switch (context->active_render_api) {
	case VULKAN_RENDER_API: {
		return Vulkan_Initialize(&context->vulkan);
	} break;
	}
	Invalid_Code_Path();
}
