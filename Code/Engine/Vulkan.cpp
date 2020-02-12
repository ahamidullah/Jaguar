#if defined(USE_VULKAN_RENDER_API)

#include "Vulkan.h"

// @TODO: Move this to render.c.
#define SHADOW_MAP_WIDTH  1024
#define SHADOW_MAP_HEIGHT 1024
#define SHADOW_MAP_INITIAL_LAYOUT GPU_IMAGE_LAYOUT_UNDEFINED
#define SHADOW_MAP_FORMAT GPU_FORMAT_D16_UNORM
#define SHADOW_MAP_IMAGE_USAGE_FLAGS ((GPU_Image_Usage_Flags)(GPU_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT | GPU_IMAGE_USAGE_SAMPLED))
#define SHADOW_MAP_SAMPLE_COUNT_FLAGS GPU_SAMPLE_COUNT_1
// Depth bias and slope are used to avoid shadowing artifacts.
// Constant depth bias factor is always applied.
#define SHADOW_MAP_CONSTANT_DEPTH_BIAS 0.25
// Slope depth bias factor is applied depending on the polygon's slope.
#define SHADOW_MAP_SLOPE_DEPTH_BIAS 1.25
#define SHADOW_MAP_FILTER GPU_LINEAR_FILTER

#define DEPTH_BUFFER_INITIAL_LAYOUT GPU_IMAGE_LAYOUT_UNDEFINED
#define DEPTH_BUFFER_FORMAT GPU_FORMAT_D32_SFLOAT_S8_UINT
#define DEPTH_BUFFER_IMAGE_USAGE_FLAGS GPU_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT
#define DEPTH_BUFFER_SAMPLE_COUNT_FLAGS GPU_SAMPLE_COUNT_1

// @TODO: Make sure that all failable Vulkan calls are VK_CHECK'd.
// @TODO: Do eg.
//        typedef VImageLayout GPU_Image_Layout;
//        #define GPU_IMAGE_LAYOUT_1 VK_IMAGE_LAYOUT_1
//        etc... That will save us from having to convert between the two values.
// @TODO: Some kind of memory protection for the GPU buffer!
// @TODO: Better texture descriptor set update scheme.
// @TODO: Move any uniform data that's updated per-frame to shared memory.
// @TODO: Scene stuff should be pbr?
// @TODO: Shouldn't per-swapchain resources actually be per-frame?
// @TODO: Query available VRAM and allocate based on that.
// @TODO: Switch to dynamic memory segments for vulkan GPU memory.
//        If more VRAM becomes available while the game is running, the game should use it!
// @TODO: Change 'model' to local? E.g. 'model_to_world_space' -> 'local_to_world_space'.
// @TODO: Avoid VK_SHARING_MODE_CONCURRENT? "Go for VK_SHARING_MODE_EXCLUSIVE and do explicit queue family ownership barriers."
// @TODO: Dedicated transfer queue.
// @TODO: Experiment with HOST_CACHED memory?
// @TODO: Keep memory mapped persistently.
// @TODO: Use a single allocation and sub-allocations for image memory.
// @TODO: Different memory management system for shared allocations.
//        Main vertex, index, image = dynamic allocator, device memory
//        Uniforms = fixed size with offsets, device memory
//        Staging = stack allocator, shared memory, per-stage lifetime
//        Per-frame memory (debug/gui vertices/indices/uvs) = stack allocator, shared memory, per-frame lifetime
//        (Pool of blocks for shared memory)
// @TODO: GPU memory defragmenting.
// @TODO: Debug clear freed memory?
// @TODO: Debug memory protection?
// @TODO: Read image pixels and mesh verices/indices directly into GPU accessable staging memory.
// @TODO: Move vulkan_context into game_state somehow.
// @TODO: What happens if MAX_FRAMES_IN_FLIGHT is less than or greater than the number of swapchain images?
// @TODO: Move logging out of band?

struct {
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	u32 graphicsQueueFamily;
	u32 presentQueueFamily;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
} vulkanContext;

#define VK_CHECK(x)\
	do {\
		VkResult _result = (x);\
		if (_result != VK_SUCCESS) Abort("VK_CHECK failed on '%s': %s\n", #x, VkResultToString(_result));\
	} while (0)

const char *VkResultToString(VkResult result) {
	switch(result) {
		case (VK_SUCCESS):
			return "VK_SUCCESS";
		case (VK_NOT_READY):
			return "VK_NOT_READY";
		case (VK_TIMEOUT):
			return "VK_TIMEOUT";
		case (VK_EVENT_SET):
			return "VK_EVENT_SET";
		case (VK_EVENT_RESET):
			return "VK_EVENT_RESET";
		case (VK_INCOMPLETE):
			return "VK_INCOMPLETE";
		case (VK_ERROR_OUT_OF_HOST_MEMORY):
			return "VK_ERROR_OUT_OF_HOST_MEMORY";
		case (VK_ERROR_INITIALIZATION_FAILED):
			return "VK_ERROR_INITIALIZATION_FAILED";
		case (VK_ERROR_DEVICE_LOST):
			return "VK_ERROR_DEVICE_LOST";
		case (VK_ERROR_MEMORY_MAP_FAILED):
			return "VK_ERROR_MEMORY_MAP_FAILED";
		case (VK_ERROR_LAYER_NOT_PRESENT):
			return "VK_ERROR_LAYER_NOT_PRESENT";
		case (VK_ERROR_EXTENSION_NOT_PRESENT):
			return "VK_ERROR_EXTENSION_NOT_PRESENT";
		case (VK_ERROR_FEATURE_NOT_PRESENT):
			return "VK_ERROR_FEATURE_NOT_PRESENT";
		case (VK_ERROR_INCOMPATIBLE_DRIVER):
			return "VK_ERROR_INCOMPATIBLE_DRIVER";
		case (VK_ERROR_TOO_MANY_OBJECTS):
			return "VK_ERROR_TOO_MANY_OBJECTS";
		case (VK_ERROR_FORMAT_NOT_SUPPORTED):
			return "VK_ERROR_FORMAT_NOT_SUPPORTED";
		case (VK_ERROR_FRAGMENTED_POOL):
			return "VK_ERROR_FRAGMENTED_POOL";
		case (VK_ERROR_OUT_OF_DEVICE_MEMORY):
			return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
		case (VK_ERROR_OUT_OF_POOL_MEMORY):
			return "VK_ERROR_OUT_OF_POOL_MEMORY";
		case (VK_ERROR_INVALID_EXTERNAL_HANDLE):
			return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
		case (VK_ERROR_SURFACE_LOST_KHR):
			return "VK_ERROR_SURFACE_LOST_KHR";
		case (VK_ERROR_NATIVE_WINDOW_IN_USE_KHR):
			return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
		case (VK_SUBOPTIMAL_KHR):
			return "VK_SUBOPTIMAL_KHR";
		case (VK_ERROR_OUT_OF_DATE_KHR):
			return "VK_ERROR_OUT_OF_DATE_KHR";
		case (VK_ERROR_INCOMPATIBLE_DISPLAY_KHR):
			return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
		case (VK_ERROR_VALIDATION_FAILED_EXT):
			return "VK_ERROR_VALIDATION_FAILED_EXT";
		case (VK_ERROR_INVALID_SHADER_NV):
			return "VK_ERROR_INVALID_SHADER_NV";
		case (VK_ERROR_NOT_PERMITTED_EXT):
			return "VK_ERROR_NOT_PERMITTED_EXT";
		case (VK_RESULT_RANGE_SIZE):
			return "VK_RESULT_RANGE_SIZE";
		case (VK_RESULT_MAX_ENUM):
			return "VK_RESULT_MAX_ENUM";
		case (VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT):
			return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
		case (VK_ERROR_FRAGMENTATION_EXT):
			return "VK_ERROR_FRAGMENTATION_EXT";
		case (VK_ERROR_INVALID_DEVICE_ADDRESS_EXT):
			return "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT";
	}
	return "Unknown VkResult Code";
}

#define VK_EXPORTED_FUNCTION(name) PFN_##name name = NULL;
#define VK_GLOBAL_FUNCTION(name) PFN_##name name = NULL;
#define VK_INSTANCE_FUNCTION(name) PFN_##name name = NULL;
#define VK_DEVICE_FUNCTION(name) PFN_##name name = NULL;
#include "VulkanFunctions.h"
#undef VK_EXPORTED_FUNCTION
#undef VK_GLOBAL_FUNCTION
#undef VK_INSTANCE_FUNCTION
#undef VK_DEVICE_FUNCTION

#include "vulkan_generated.c"

#define VULKAN_MEMORY_BLOCK_SIZE MEGABYTE(256)

// GPU Memory
#define VULKAN_VERTEX_MEMORY_SEGMENT_SIZE MEGABYTE(50)
#define VULKAN_INDEX_MEMORY_SEGMENT_SIZE MEGABYTE(50)
#define VULKAN_UNIFORM_MEMORY_SEGMENT_SIZE MEGABYTE(50)
#define VULKAN_INSTANCE_MEMORY_SEGMENT_SIZE MEGABYTE(50)
#define VULKAN_GPU_ALLOCATION_SIZE (VULKAN_VERTEX_MEMORY_SEGMENT_SIZE + VULKAN_INDEX_MEMORY_SEGMENT_SIZE + VULKAN_UNIFORM_MEMORY_SEGMENT_SIZE + VULKAN_INSTANCE_MEMORY_SEGMENT_SIZE)

// @TODO: Move per-frame uniforms to the shared memory segment?
// Shared Memory
#define VULKAN_FRAME_VERTEX_MEMORY_SEGMENT_SIZE MEGABYTE(50)
#define VULKAN_FRAME_INDEX_MEMORY_SEGMENT_SIZE MEGABYTE(50)
#define VULKAN_STAGING_MEMORY_SEGMENT_SIZE MEGABYTE(50)
#define VULKAN_STAGING_MEMORY_SIZE MEGABYTE(50)
#define VULKAN_SHARED_ALLOCATION_SIZE ((GPU_MAX_FRAMES_IN_FLIGHT * VULKAN_FRAME_VERTEX_MEMORY_SEGMENT_SIZE) + (GPU_MAX_FRAMES_IN_FLIGHT * VULKAN_FRAME_INDEX_MEMORY_SEGMENT_SIZE) + VULKAN_STAGING_MEMORY_SEGMENT_SIZE)

// Image Memory
#define VULKAN_IMAGE_ALLOCATION_SIZE MEGABYTE(400)

#define VULKAN_VERTEX_MEMORY_SEGMENT_OFFSET 0
#define VULKAN_INDEX_MEMORY_SEGMENT_OFFSET (VULKAN_VERTEX_MEMORY_SEGMENT_OFFSET + VULKAN_VERTEX_MEMORY_SEGMENT_SIZE)
#define VULKAN_UNIFORM_MEMORY_SEGMENT_OFFSET (VULKAN_INDEX_MEMORY_SEGMENT_OFFSET + VULKAN_INDEX_MEMORY_SEGMENT_SIZE)
// ?
#define VULKAN_INSTANCE_MEMORY_SEGMENT_OFFSET (VULKAN_UNIFORM_MEMORY_SEGMENT_OFFSET + VULKAN_UNIFORM_MEMORY_SEGMENT_SIZE)
#define VULKAN_STAGING_MEMORY_SEGMENT_OFFSET 0
#define VULKAN_FRAME_VERTEX_MEMORY_SEGMENT_OFFSET (VULKAN_STAGING_MEMORY_SEGMENT_SIZE)
#define VULKAN_FRAME_INDEX_MEMORY_SEGMENT_OFFSET (VULKAN_FRAME_VERTEX_MEMORY_SEGMENT_OFFSET + (GPU_MAX_FRAMES_IN_FLIGHT * VULKAN_FRAME_VERTEX_MEMORY_SEGMENT_SIZE))

#define VULKAN_MAX_TEXTURES 100
#define VULKAN_MAX_MATERIALS 100

#define GPU_VERTEX_BUFFER_BIND_ID 0
#define GPU_INSTANCE_BUFFER_BIND_ID 1

struct Instance_Data {
	u32 material_id;
};

typedef struct {
	VkShaderModule module;
	VkShaderStageFlagBits stage;
} Shader_Module;

typedef struct {
	u32            module_count;
	Shader_Module *modules;
} Shaders;

typedef struct {
	VkImage        image;
	VkImageView    image_view;
	VkDeviceMemory memory;
} Framebuffer_Attachment;

typedef struct {
	VkImage     images[VULKAN_MAX_TEXTURES];
	VkImageView image_views[VULKAN_MAX_TEXTURES];
	u32         image_memory_offsets[VULKAN_MAX_TEXTURES];
	//TextureID   id_generator;
	u32         count;
} GPU_Textures;

typedef struct {
	s32 normal_map;
	s32 material_albedo_map;
	s32 material_normal_map;
	s32 material_metallic_map;
	s32 material_roughness_map;
	s32 material_ao_map;
} Push_Constants;

// @TODO: Should this be merged with the regular game material?
typedef struct {
	u32 albedo_map;
	u32 normal_map;
	u32 metallic_map;
	u32 roughness_map;
	u32 ambient_occlusion_map;
	u32 padding[3];
} Vulkan_Material;

#define MAX_SHADER_MODULES 3

/*
typedef struct {
	VkRenderPass render_pass;
	VkPipelineLayout pipeline_layout;
	VkPipeline pipeline;
	struct {
		VkShaderModule module;
		VkShaderStageFlagBits stage;
	} modules[MAX_SHADER_MODULES];
	u32 module_count;
} Shader;
*/

enum {
	SCENE_SAMPLER_DESCRIPTOR_SET = 0,
	SCENE_UNIFORM_DESCRIPTOR_SET,
	SCENE_TEXTURE_DESCRIPTOR_SET,
	SCENE_DYNAMIC_UNIFORM_DESCRIPTOR_SET,
	SCENE_MATERIAL_DESCRIPTOR_SET,
	SCENE_DESCRIPTOR_SET_TYPE_COUNT,

	SHADOW_MAP_UNIFORM_DESCRIPTOR_SET = 0,
	SHADOW_MAP_DESCRIPTOR_SET_TYPE_COUNT,

	FLAT_COLOR_UNIFORM_DESCRIPTOR_SET = 0,
	FLAT_COLOR_DESCRIPTOR_SET_TYPE_COUNT,
};

struct _Render_API_Context {
	VkDebugUtilsMessengerEXT  debug_messenger;
	VkInstance                instance;
	VkPhysicalDevice          physical_device;
	VkDevice                  device;
	VkSurfaceKHR              surface;
	VkSurfaceFormatKHR        surface_format;
	VkPresentModeKHR          present_mode;
	u32                       graphics_queue_family;
	u32                       present_queue_family;
	VkQueue                   graphics_queue;
	VkQueue                   present_queue;
	VkSwapchainKHR            swapchain;
	VkExtent2D                swapchain_image_extent;
	u32                       num_swapchain_images;
	VkImageView               swapchain_image_views[3];
	VkFramebuffer             framebuffers[3];
	VkRenderPass              render_pass;
	VkPipelineLayout          pipeline_layout;
	VkDescriptorPool          descriptor_pool;
	VkCommandPool             command_pool;
	VkCommandBuffer           command_buffers[3];
	VkSemaphore               image_available_semaphores[GPU_MAX_FRAMES_IN_FLIGHT];
	VkSemaphore               render_finished_semaphores[GPU_MAX_FRAMES_IN_FLIGHT];
	VkFence                   inFlightFences[GPU_MAX_FRAMES_IN_FLIGHT];
	VkFence                   assetFences[GPU_MAX_FRAMES_IN_FLIGHT];
	u32                       currentFrame;
	u32                       nextFrame;
	GPU_Textures              textures;
	VkSampler                 textureSampler;
	VkImage                   depthImage;
	VkDeviceMemory            depthImageMemory;
	VkImageView               depthImageView;
	VkDeviceMemory            shared_memory;
	VkDeviceMemory            gpu_memory;
	//GPU_Memory_Allocator _gpu_memory;
	//GPU_Memory_Allocator _shared_memory;
	//GPU_Memory_Allocation *_staging_memory;
	u32 _staging_memory_bytes_used;
	VkDeviceMemory            image_memory;
	u32                       image_memory_bytes_used;
	u32                       vertex_memory_bytes_used;
	u32                       index_memory_bytes_used;
	u32                       staging_memory_bytes_used;
	u32                       debug_vertex_memory_bytes_used; // @TODO: Frame?
	u32                       debug_index_memory_bytes_used; // @TODO: Frame?
	u32                       vertex_count;
	u32                       index_count;
	u32                       debug_vertex_count; // @TODO: Frame?
	u32                       debug_index_count; // @TODO: Frame?
	VkBuffer                  gpu_buffer;
	VkBuffer                  staging_buffer;
	Shaders                   shaders[GPU_SHADER_COUNT];
	VkDeviceSize              minimum_uniform_buffer_offset_alignment; // Any uniform or dynamic uniform buffer's offset inside a Vulkan memory block must be a multiple of this byte count.
	VkDeviceSize              maximum_uniform_buffer_size;             // Maximum size of any uniform buffer (including dynamic uniform buffers). @TODO: Move to sizes struct?
	//Memory_Arena             *arena;
	PlatformMutex mutex;

	struct {
		// @TODO: Move vertex, index, uniform starts/frontiers into here.
		u32 instance_memory_segment;
		u32 vertex_memory_segment;
		u32 index_memory_segment;
		u32 uniform_memory_segment;
		struct {
			u32 materials;
			u32 uniform[3];
			u32 dynamic_uniform[3];
		} scene;
		struct {
			u32 uniform;
		} shadow_map;
		struct {
			u32 uniform;
		} flat_color;
	} buffer_offsets;

	struct {
		// @TODO: Move vertex, index, uniform sizes into here.
		u32 instance_memory_segment;
		u32 vertex_memory_segment;
		u32 index_memory_segment;
		u32 uniform_memory_segment;
		struct {
			u32 aligned_ubo;
			u32 aligned_dynamic_ubo;
			u32 dynamic_uniform_buffer;
		} scene;
	} sizes;

	struct {
	} limits;

	struct {
		struct {
			VkDescriptorSet sampler;
			VkDescriptorSet texture;
			VkDescriptorSet uniform[3];
			VkDescriptorSet dynamic_uniform[3];
			VkDescriptorSet materials;
		} scene;
		struct {
			VkDescriptorSet uniform;
		} shadow_map;
		struct {
			VkDescriptorSet uniform;
		} flat_color;
	} descriptor_sets;

	struct {
		VkDescriptorSetLayout scene[SCENE_DESCRIPTOR_SET_TYPE_COUNT];
		VkDescriptorSetLayout shadow_map[SHADOW_MAP_DESCRIPTOR_SET_TYPE_COUNT];
		VkDescriptorSetLayout flat_color[FLAT_COLOR_DESCRIPTOR_SET_TYPE_COUNT];
	} descriptor_set_layouts;

	struct {
		VkPipelineLayout shadow_map;
		VkPipelineLayout flat_color;
	} pipeline_layouts;

	struct {
		VkPipeline textured_static;
		VkPipeline shadow_map_static;
		VkPipeline flat_color;
	} pipelines;

	struct {
		VkRenderPass shadow_map;
		VkRenderPass flat_color;
	} render_passes;

	struct {
		VkFramebuffer shadow_map;
	} xframebuffers;

	struct {
		Framebuffer_Attachment shadow_map_depth;
	} framebuffer_attachments;

	struct {
		VkSampler shadow_map_depth;
	} samplers;
};

typedef struct {
	V3 camera_position;
} Scene_UBO;

//typedef struct {
	//V4 color;
	//M4 model_view_projection;
//} Flat_Color_UBO;

// @TODO: Fix name.
#define TEST_INSTANCES 5

#define alignas(x)

typedef struct {
	alignas(16) M4 model_to_world_space;
	alignas(16) M4 world_to_clip_space;
	alignas(16) M4 world_to_shadow_map_clip_space;
} Dynamic_Scene_UBO;

Dynamic_Scene_UBO dynamic_scene_ubo[TEST_INSTANCES];

typedef struct {
	alignas(16) M4 world_to_clip_space;
} Shadow_Map_UBO;

Shadow_Map_UBO shadow_map_ubo;

u32 Align_U32(u32 number, u32 alignment) {
	u32 remainder = number % alignment;
	if (remainder == 0) {
		return number;
	}
	return number + alignment - remainder;
}

u32 align_to(u32 size, u32 alignment) {
	u32 remainder = size % alignment;
	if (remainder == 0) {
		return size;
	}
	return size + alignment - remainder;
}

u32 VulkanDebugMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT *callbackData, void *userData) {
	LogType logType;
	const char *severityString;
	switch (severity) {
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: {
		logType = INFO_LOG;
		severityString = "Info";
	} break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
		logType = ERROR_LOG;
		severityString = "Warning";
	} break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
		logType = ERROR_LOG;
		severityString = "Error";
	} break;
	default: {
		logType = INFO_LOG;
		severityString = "Unknown";
	};
	}
	const char *typeString;
	switch (type) {
	case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: {
		typeString = "General";
	} break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: {
		typeString = "Validation";
	} break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: {
		typeString = "Performance";
	} break;
	default: {
		typeString = "Unknown";
	};
	}
	LogPrint(logType, "Vulkan debug message: %s: %s: %s\n", severityString, typeString, callbackData->pMessage);
	if (logType == ERROR_LOG) {
		PlatformPrintStacktrace();
	}
    return 0;
}

VkCommandBuffer Render_API_Create_Command_Buffer(VkCommandPool commandPool) {
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
    };
	VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(vulkanContext.device, &commandBufferAllocateInfo, &commandBuffer);
	VkCommandBufferBeginInfo commandBufferBeginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};
	vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
	return commandBuffer;
}

void Render_API_Submit_Command_Buffers(u32 count, VkCommandBuffer *commandBuffers, GPU_Command_Queue_Type queueType, VkFence fence) {
	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = count,
		.pCommandBuffers = commandBuffers,
	};
	switch (queueType) {
	case GPU_GRAPHICS_COMMAND_QUEUE: {
		VK_CHECK(vkQueueSubmit(vulkanContext.graphicsQueue, 1, &submit_info, fence));
	} break;
	default: {
		InvalidCodePath();
	} break;
	}
}

void Render_API_Free_Command_Buffers(Render_API_Context *context, GPU_Command_Pool command_pool, s32 command_buffer_count, GPU_Command_Buffer *command_buffers) {
	vkFreeCommandBuffers(context->device, command_pool, command_buffer_count, command_buffers);
}

void TEMPORARY_VULKAN_SUBMIT(Render_API_Context *context, GPU_Command_Buffer command_buffer, s32 current_frame_index, GPU_Fence fence) {
	VkSemaphore wait_semaphores[] = {context->image_available_semaphores[current_frame_index]};
	VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSemaphore signal_semaphores[] = {context->render_finished_semaphores[current_frame_index]};
	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = ArrayCount(wait_semaphores),
		.pWaitSemaphores = wait_semaphores,
		.pWaitDstStageMask = wait_stages,
		.commandBufferCount = 1,
		.pCommandBuffers = &command_buffer,
		.signalSemaphoreCount = ArrayCount(signal_semaphores),
		.pSignalSemaphores = signal_semaphores,
	};
	VK_CHECK(vkQueueSubmit(context->graphics_queue, 1, &submit_info, fence));
}

void Render_API_End_Command_Buffer(VkCommandBuffer commandBuffer) {
	VK_CHECK(vkEndCommandBuffer(commandBuffer));
}

VkCommandPool RenderAPICreateCommandPool(GPU_Command_Queue_Type queueType) {
	VkCommandPoolCreateInfo commandPoolCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		//.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
	};
	switch (queueType) {
	case GPU_GRAPHICS_COMMAND_QUEUE: {
		commandPoolCreateInfo.queueFamilyIndex = vulkanContext.graphicsQueueFamily;
	} break;
	default: {
		InvalidCodePath();
	} break;
	}
	VkCommandPool pool;
	VK_CHECK(vkCreateCommandPool(vulkanContext.device, &commandPoolCreateInfo, NULL, &pool));
	return pool;
}

void Render_API_Reset_Command_Pool(Render_API_Context *context, VkCommandPool command_pool) {
	vkResetCommandPool(context->device, command_pool, 0);
}

void Render_API_Record_Copy_Buffer_Command(VkCommandBuffer commandBuffer, u32 size, VkBuffer source, VkBuffer destination, u32 sourceOffset, u32 destinationOffset) {
	VkBufferCopy bufferCopy = {
		.srcOffset = sourceOffset,
		.dstOffset = destinationOffset,
		.size = size,
	};
	vkCmdCopyBuffer(commandBuffer, source, destination, 1, &bufferCopy);
}

GPU_Resource_Allocation_Requirements Render_API_Get_Buffer_Allocation_Requirements(GPU_Buffer buffer) {
	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(vulkanContext.device, buffer, &memoryRequirements);
	return memoryRequirements;
}

VkBuffer Render_API_Create_Buffer(u32 size, VkBufferUsageFlags usageFlags) {
	VkBufferCreateInfo bufferCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = size,
		.usage = usageFlags,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};
	VkBuffer buffer;
	VK_CHECK(vkCreateBuffer(vulkanContext.device, &bufferCreateInfo, NULL, &buffer));
	return buffer;
}

void Render_API_Bind_Buffer_Memory(GPU_Buffer buffer, GPU_Memory memory, u32 memoryOffset) {
	VK_CHECK(vkBindBufferMemory(vulkanContext.device, buffer, memory, memoryOffset));
}

void Render_API_Destroy_Buffer(Render_API_Context *context, GPU_Buffer buffer) {
	vkDestroyBuffer(context->device, buffer, NULL);
}

void *Render_API_Map_Memory(VkDeviceMemory memory, u32 size, u32 offset) {
	void *pointer;
	VK_CHECK(vkMapMemory(vulkanContext.device, memory, offset, size, 0, &pointer));
	return pointer;
}

bool Render_API_Allocate_Memory(u32 size, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceMemory *memory) {
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(vulkanContext.physicalDevice, &physicalDeviceMemoryProperties);
	s32 selectedMemoryTypeIndex = -1;
	for (u32 i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++) {
		if ((physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags) {
			selectedMemoryTypeIndex = i;
			break;
		}
	}
	if (selectedMemoryTypeIndex < 0) {
		Abort("Failed to find suitable GPU memory type");
	}
	VkMemoryAllocateInfo memoryAllocateInfo = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = size,
		.memoryTypeIndex = (u32)selectedMemoryTypeIndex,
	};
	VkResult result = vkAllocateMemory(vulkanContext.device, &memoryAllocateInfo, NULL, memory);
	if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
		return false;
	}
	VK_CHECK(result);
	return true;
}

VkFence RenderAPICreateFence(bool startSignalled) {
	VkFenceCreateInfo fenceCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = 0,
	};
	if (startSignalled)
	{
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	}
	VkFence fence;
	vkCreateFence(vulkanContext.device, &fenceCreateInfo, NULL, &fence);
	return fence;
}

bool Render_API_Was_Fence_Signalled(VkFence fence) {
	VkResult result = vkGetFenceStatus(vulkanContext.device, fence);
	if (result == VK_SUCCESS) {
		return true;
	}
	if (result == VK_NOT_READY) {
		return false;
	}
	VK_CHECK(result);
	return false;
}

void Render_API_Wait_For_Fences(u32 count, VkFence *fences, bool wait_for_all_fences, u64 timeout) {
	VK_CHECK(vkWaitForFences(vulkanContext.device, count, fences, wait_for_all_fences, timeout));
}

void Render_API_Reset_Fences(u32 count, VkFence *fences) {
	VK_CHECK(vkResetFences(vulkanContext.device, count, fences));
}

u32 Render_API_Acquire_Next_Swapchain_Image_Index(Render_API_Context *context, GPU_Swapchain swapchain, u32 current_frame_index) {
	u32 swapchain_image_index = 0;
	VK_CHECK(vkAcquireNextImageKHR(context->device, swapchain, UINT64_MAX, context->image_available_semaphores[current_frame_index], NULL, &swapchain_image_index));
	return swapchain_image_index;
}

#if 0
u8 allocate_vulkan_memory(VkDeviceMemory *memory, VkMemoryRequirements memory_requirements, VkMemoryPropertyFlags desired_memory_properties) {
	VkMemoryAllocateInfo memory_allocate_info = {
		.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memory_requirements.size,
	};
	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(vulkan_context.physical_device, &memory_properties);
	s32 selected_memory_type_index = -1;
	for (u32 i = 0; i < memory_properties.memoryTypeCount; i++) {
		if ((memory_requirements.memoryTypeBits & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & desired_memory_properties) == desired_memory_properties) {
			selected_memory_type_index = i;
			break;
		}
	}
	if (selected_memory_type_index < 0) {
		_abort("Failed to find suitable GPU memory type");
	}
	memory_allocate_info.memoryTypeIndex = selected_memory_type_index;
	VkResult result = vkAllocateMemory(vulkan_context.device, &memory_allocate_info, NULL, memory);
	if (result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			return 0;
		}
		VK_CHECK(result);
	}
	return 1;
}

u8 allocate_vulkan_memory_block(GPU_Memory_Allocator *allocator) { ///Vulkan_Memory_Block *block, VkBufferUsageFlags buffer_usage, VkMemoryPropertyFlags desired_memory_properties) {
	Vulkan_Memory_Block *new_block = allocate_struct(vulkan_context.arena, Vulkan_Memory_Block);
	VkBufferCreateInfo buffer_create_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = VULKAN_MEMORY_BLOCK_SIZE,
		.usage = allocator->buffer_usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};
	VK_CHECK(vkCreateBuffer(vulkan_context.device, &buffer_create_info, NULL, &new_block->buffer));
	if (allocator->base_block) {
		allocator->active_block->next = new_block;
		allocator->active_block = new_block;
	} else {
		allocator->base_block = new_block;
		allocator->active_block = allocator->base_block;
		vkGetBufferMemoryRequirements(vulkan_context.device, new_block->buffer, &allocator->memory_requirements);
	}
	if (!allocate_vulkan_memory(&new_block->device_memory, allocator->memory_requirements, allocator->memory_properties)) {
		return 0;
	}
	VK_CHECK(vkBindBufferMemory(vulkan_context.device, new_block->buffer, new_block->device_memory, 0));
	if (allocator->memory_properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
		VK_CHECK(vkMapMemory(vulkan_context.device, new_block->device_memory, 0, VK_WHOLE_SIZE, 0, &new_block->mapped_pointer));
	} else {
		new_block->mapped_pointer = NULL;
	}
	new_block->frontier = 0;
	new_block->freed_allocation_count = 0;
	new_block->active_allocation_count = 0;
	new_block->next = NULL;
	return 1;
}

u8 initialize_vulkan_dynamic_memory_allocator(GPU_Memory_Allocator *allocator, VkBufferUsageFlags buffer_usage, VkMemoryPropertyFlags memory_properties) {
	allocator->buffer_usage = buffer_usage;
	allocator->memory_properties = memory_properties;
	return allocate_vulkan_memory_block(allocator);
}

/*
u32 try_to_allocate_vulkan_memory_blocks(u32 size) {
	assert(size < VULKAN_MEMORY_CHUNK_SIZE);
	u32 required_block_count = divide_and_round_up(size, VULKAN_MEMORY_BLOCK_SIZE);
}

u32 try_to_allocate_vulkan_gpu_memory_chunks(u32 count) {
	if (!vulkan_context.base_gpu_memory_chunk) {
		vulkan_context.base_gpu_memory_chunk = allocate_struct(vulkan_context.arena, Vulkan_Memory_Chunk);
		vulkan_context.active_gpu_memory_chunk = vulkan_context.base_gpu_memory_chunk;
	}
	return try_to_allocate_vulkan_memory_chunk(&vulkan_context.active_gpu_memory_chunk, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

u32 try_to_allocate_vulkan_shared_memory_chunk(u32 count) {
	if (!vulkan_context.base_shared_memory_chunk) {
		vulkan_context.base_shared_memory_chunk = allocate_struct(vulkan_context.arena, Vulkan_Memory_Chunk);
		vulkan_context.active_shared_memory_chunk = vulkan_context.base_shared_memory_chunk;
	}
	return try_to_allocate_vulkan_memory_chunk(&vulkan_context.active_shared_memory_chunk, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT  | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}
*/

GPU_Memory_Allocation *acquire_vulkan_memory(GPU_Memory_Allocator *allocator, u32 size) {
	for (u32 i = 0; i < allocator->active_block->freed_allocation_count; i++) {
		Assert(0);
	}
	GPU_Memory_Allocation *new_allocation = &allocator->active_block->active_allocations[allocator->active_block->active_allocation_count];
	*new_allocation = (GPU_Memory_Allocation){
		.block          = allocator->active_block,
		.size           = size,
		.mapped_pointer = allocator->active_block->mapped_pointer,
		.buffer         = allocator->active_block->buffer,
		.offset         = allocator->active_block->frontier,
	};
	allocator->active_block->active_allocation_count++;
	Assert(allocator->active_block->active_allocation_count < VULKAN_MAX_MEMORY_ALLOCATIONS_PER_BLOCK);
	allocator->active_block->frontier += size;
	Assert(allocator->active_block->frontier < VULKAN_MEMORY_BLOCK_SIZE);
	return new_allocation;
}

void release_vulkan_memory(GPU_Memory_Allocation *memory) {
	memory->block->freed_allocation_sizes[memory->block->freed_allocation_count] = memory->size;
	memory->block->freed_allocation_offsets[memory->block->freed_allocation_count] = memory->offset;
	memory->block->freed_allocation_count++;
	memory->block->active_allocation_count--;
	if (memory->block->active_allocation_count > 0) {
		*memory = memory->block->active_allocations[memory->block->active_allocation_count];
	}
}

void _stage_vulkan_data(void *data, u32 size) {
	//Vulkan_Memory_Allocation *memory = acquire_vulkan_memory(&vulkan_context._shared_memory, size);
	//if (!memory) {
		//assert(0); // @TODO
	//}

	memcpy((char *)vulkan_context._staging_memory->mapped_pointer + vulkan_context._staging_memory_bytes_used, data, size);
	vulkan_context._staging_memory_bytes_used += size;
	Assert(vulkan_context._staging_memory_bytes_used < VULKAN_STAGING_MEMORY_SIZE);
}

GPU_Memory_Allocation *upload_staged_vulkan_data() {
/*
	Vulkan_Memory_Allocation *gpu_memory = acquire_vulkan_memory(&vulkan_context._gpu_memory, vulkan_context._staging_memory_bytes_used);
	if (!gpu_memory) {
		Assert(0); // @TODO
	}
	// @TODO: Try to create a reusable command buffer for copying?
    VkCommandBufferAllocateInfo command_buffer_allocate_info = {
		.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandPool        = vulkan_context.command_pool,
		.commandBufferCount = 1,
    };
    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(vulkan_context.device, &command_buffer_allocate_info, &command_buffer);
	VkCommandBufferBeginInfo command_buffer_begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};
	vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);
	VkBufferCopy copy_region = {
		.srcOffset = vulkan_context._staging_memory->offset,
		.dstOffset = gpu_memory->offset,
		.size      = vulkan_context._staging_memory_bytes_used,
	};
	vkCmdCopyBuffer(command_buffer, vulkan_context._staging_memory->buffer, gpu_memory->buffer, 1, &copy_region);
	vkEndCommandBuffer(command_buffer);
	Atomic_Write_To_Ring_Buffer(vulkan_context.asset_upload_command_buffers, command_buffer);
	//VkSubmitInfo submit_info = {
		//.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		//.commandBufferCount = 1,
		//.pCommandBuffers    = &command_buffer,
	//};
	//VK_CHECK(vkQueueSubmit(vulkan_context.graphics_queue, 1, &submit_info, VK_NULL_HANDLE));
	//VK_CHECK(vkQueueWaitIdle(vulkan_context.graphics_queue)); // @TODO: Use a fence?
	//vkFreeCommandBuffers(vulkan_context.device, vulkan_context.command_pool, 1, &command_buffer);
	//vulkan_context._staging_memory_bytes_used = 0;
	//vulkan_context.staging_memory_bytes_used = 0;
	return gpu_memory;
*/
	return NULL;
}

GPU_Memory_Allocation *stage_and_upload_data(void *data, u32 size) {
	//Platform_Lock_Mutex(&vulkan_context.mutex);
	_stage_vulkan_data(data, size);
	GPU_Memory_Allocation *gpu_memory = upload_staged_vulkan_data();
	//Platform_Unlock_Mutex(&vulkan_context.mutex);
	//release_vulkan_memory(staging_memory);
	return gpu_memory;

	//vulkan_context.staging_memory_bytes_used += size;
	//upload_vulkan_data(data, size, staging_memory);
	/*
	if (vulkan_context.active_gpu_memory_chunk.used_block_count + required_block_count > VULKAN_MEMORY_BLOCKS_PER_CHUNK) {
		u32 number_of_unused_blocks_in_chunk = VULKAN_MEMORY_BLOCKS_PER_CHUNK - vulkan_context.active_gpu_memory_chunk.used_block_count;
		u32 required_chunk_count = divide_and_round_up(required_block_count - number_of_unused_blocks_in_chunk, VULKAN_MEMORY_CHUNK_SIZE);
		for (u32 i = 0; i < 
	}
	*/
}

void stage_vulkan_data(void *data, u32 size);
void transfer_staged_vulkan_data(u32 offset);

/*
GPU_Mesh GPU_Upload_Mesh(u32 vertex_count, u32 sizeof_vertex, void *vertices, u32 index_count, u32 *indices) {
	// @TODO: Multithreaded asset load?

	return (GPU_Mesh){
		.vertices = stage_and_upload_data(vertices, vertex_count * sizeof_vertex),
		.indices = stage_and_upload_data(indices, index_count * sizeof(u32)),
	};

	//u32 total_vertex_count = 0;
	//for (s32 i = 0; i < model->mesh_count; i++) {
		//stage_and_upload_vertex_data(mesh->vertices, mesh->vertex_count * sizeof(Vertex));
		//stage_vulkan_data(mesh->vertices, mesh->vertex_count * sizeof(Vertex));
		//total_vertex_count += model->meshes[i].vertex_count;
	//}
	//transfer_staged_vulkan_data(VULKAN_VERTEX_MEMORY_SEGMENT_OFFSET + vulkan_context.vertex_memory_bytes_used);
	//vulkan_context.vertex_memory_bytes_used += mesh->vertex_count * sizeof(Vertex);
	//ASSERT(vulkan_context.vertex_memory_bytes_used < VULKAN_VERTEX_MEMORY_SEGMENT_SIZE);
	// *vertex_offset = vulkan_context.vertex_count;
	//vulkan_context.vertex_count += mesh->vertex_count;

	// *first_index = vulkan_context.index_count;
	//u32 total_index_count = 0;
	//for (s32 i = 0; i < model->mesh_count; i++) {
		//stage_vulkan_data(mesh->indices, mesh->index_count * sizeof(u32));
		//total_index_count += model->meshes[i].index_count;
	//}
	//transfer_staged_vulkan_data(VULKAN_INDEX_MEMORY_SEGMENT_OFFSET + vulkan_context.index_memory_bytes_used);
	//vulkan_context.index_memory_bytes_used += mesh->index_count * sizeof(u32);
	//ASSERT(vulkan_context.index_memory_bytes_used < VULKAN_INDEX_MEMORY_SEGMENT_SIZE);
	//vulkan_context.index_count += mesh->index_count;
}
*/

VkCommandBuffer Create_Single_Use_Vulkan_Command_Buffer(VkCommandPool command_pool) {
	VkCommandBufferAllocateInfo command_buffer_allocate_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandPool = command_pool,
		.commandBufferCount = 1,
	};
	VkCommandBuffer command_buffer;
	vkAllocateCommandBuffers(vulkan_context.device, &command_buffer_allocate_info, &command_buffer);
	VkCommandBufferBeginInfo command_buffer_begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};
	vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);
	return command_buffer;
}

VkFence Submit_Single_Use_Vulkan_Command_Buffer(VkCommandBuffer command_buffer) {
	vkEndCommandBuffer(command_buffer);
	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &command_buffer,
	};
	VkFenceCreateInfo fence_create_info = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
	};
	VkFence fence;
	vkCreateFence(vulkan_context.device, &fence_create_info, NULL, &fence);
	vkQueueSubmit(vulkan_context.graphics_queue, 1, &submit_info, fence);
	return fence;
}

VkCommandBuffer beginSingleTimeCommands() {
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = vulkan_context.command_pool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(vulkan_context.device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(vulkan_context.graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(vulkan_context.graphics_queue);

	vkFreeCommandBuffers(vulkan_context.device, vulkan_context.command_pool, 1, &commandBuffer);
}

void Transition_Vulkan_Image_Layout(GPU_Command_List command_list, GPU_Image image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	} else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	} else {
		_abort("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, NULL,
		0, NULL,
		1, &barrier
	);
}
#endif

u32 Render_API_Get_Swapchain_Image_Count(Render_API_Context *context, VkSwapchainKHR swapchain) {
	u32 swapchain_image_count;
	VK_CHECK(vkGetSwapchainImagesKHR(context->device, swapchain, &swapchain_image_count, NULL));
	return swapchain_image_count;
}

void Render_API_Get_Swapchain_Image_Views(Render_API_Context *context, VkSwapchainKHR swapchain, u32 count, VkImageView *image_views) {
	VkImage images[count];
	VK_CHECK(vkGetSwapchainImagesKHR(context->device, swapchain, &count, images));
	for (s32 i = 0; i < count; i++) {
		VkImageViewCreateInfo image_view_create_info = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = context->window_surface_format.format,
			.components = {
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY,
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY,
			},
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};
		VK_CHECK(vkCreateImageView(context->device, &image_view_create_info, NULL, &image_views[i]));
	}
}

VkSwapchainKHR Render_API_Create_Swapchain(Render_API_Context *context) {
	VkSurfaceCapabilitiesKHR surface_capabilities;
	VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context->physical_device, context->window_surface, &surface_capabilities));
	VkExtent2D swapchain_image_extent;
	if (surface_capabilities.currentExtent.width != UINT32_MAX) {
		swapchain_image_extent = surface_capabilities.currentExtent;
	} else {
		swapchain_image_extent.width = Maximum(surface_capabilities.minImageExtent.width, Minimum(surface_capabilities.maxImageExtent.width, window_width));
		swapchain_image_extent.height = Maximum(surface_capabilities.minImageExtent.height, Minimum(surface_capabilities.maxImageExtent.height, window_height));
	}
	if (swapchain_image_extent.width != window_width && swapchain_image_extent.height != window_height) {
		Abort("Swapchain image dimensions do not match the window dimensions: swapchain %ux%u, window %ux%u", swapchain_image_extent.width, swapchain_image_extent.height, window_width, window_height);
	}
	u32 desired_swapchain_image_count = surface_capabilities.minImageCount + 1;
	if (surface_capabilities.maxImageCount > 0 && (desired_swapchain_image_count > surface_capabilities.maxImageCount)) {
		desired_swapchain_image_count = surface_capabilities.maxImageCount;
	}
	VkSwapchainCreateInfoKHR swapchain_create_info = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = context->window_surface,
		.minImageCount = desired_swapchain_image_count,
		.imageFormat = context->window_surface_format.format,
		.imageColorSpace = context->window_surface_format.colorSpace,
		.imageExtent = swapchain_image_extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform = surface_capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = context->present_mode,
		.clipped = 1,
		.oldSwapchain = NULL,
	};
	u32 queue_family_indices[] = {
		context->graphics_queue_family,
		context->present_queue_family,
	};
	if (context->graphics_queue_family != context->present_queue_family) {
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchain_create_info.queueFamilyIndexCount = ArrayCount(queue_family_indices);
		swapchain_create_info.pQueueFamilyIndices = queue_family_indices;
	} else {
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	VkSwapchainKHR swapchain;
	VK_CHECK(vkCreateSwapchainKHR(context->device, &swapchain_create_info, NULL, &swapchain));
	return swapchain;
}

// @TODO: Better abstraction for render passes.

typedef VkAttachmentDescription GPU_Framebuffer_Attachment_Description;
typedef VkSubpassDescription GPU_Subpass_Description;
typedef VkSubpassDependency GPU_Subpass_Dependency;

typedef struct GPU_Attachment_Description {
} GPU_Attachment_Description;

/*
typedef struct xRender_Pass {
	u32 subpass_dependency_count;
	GPU_Subpass_Dependency *subpass_dependencies;
	u32 subpass_count;
	GPU_Subpass_Description *subpass_descriptions;
	u32 attachment_count;
	GPU_Attachment_Description *attachment_descriptions;
} xRender_Pass;
*/

#if 0
typedef struct GPU_Render_Graph {
	u32 render_pass_count;
	VkRenderPass *render_passes;
} GPU_Render_Graph;

GPU_Render_Graph GPU_Compile_Render_Graph(Render_API_Context *context, Render_Graph_Description *render_graph_description) {
	GPU_Render_Graph render_graph = {
		.render_pass_count = 2,
		.render_passes = (VkRenderPass *)malloc(2 * sizeof(VkRenderPass)), // @TODO
	};
	{
		VkSubpassDependency subpass_dependencies[] = {
			{
				.srcSubpass      = VK_SUBPASS_EXTERNAL,
				.dstSubpass      = 0,
				.srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				.dstStageMask    = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
				.srcAccessMask   = VK_ACCESS_SHADER_READ_BIT,
				.dstAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
			},
			{
				.srcSubpass      = 0,
				.dstSubpass      = VK_SUBPASS_EXTERNAL,
				.srcStageMask    = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				.dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				.srcAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				.dstAccessMask   = VK_ACCESS_SHADER_READ_BIT,
				.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
			}
		};
		VkAttachmentDescription AttachmentDescription = {
			.format = (VkFormat)SHADOW_MAP_FORMAT,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
		};
		VkRenderPassCreateInfo render_pass_create_info = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = 1,
			.pAttachments = AttachmentDescription,
			.subpassCount = 1,
			.pSubpasses      = &(VkSubpassDescription){
				.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.colorAttachmentCount    = 0,
				.pDepthStencilAttachment = &(VkAttachmentReference){
					.attachment = 0,
					.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				},
			},
			.dependencyCount = ArrayCount(subpass_dependencies),
			.pDependencies   = subpass_dependencies,
		};
		VK_CHECK(vkCreateRenderPass(context->device, &render_pass_create_info, NULL, &render_graph.render_passes[0]));
	}
	{
		VkAttachmentDescription attachments[] = {
			{
				.format         = context->window_surface_format.format,
				.samples        = VK_SAMPLE_COUNT_1_BIT,
				.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			},
			{
				.format         = VK_FORMAT_D32_SFLOAT_S8_UINT,
				.samples        = VK_SAMPLE_COUNT_1_BIT,
				.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			},
		};
		VkRenderPassCreateInfo render_pass_create_info = {
			.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = ArrayCount(attachments),
			.pAttachments    = attachments,
			.subpassCount    = 1,
			.pSubpasses      = &(VkSubpassDescription){
				.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.colorAttachmentCount = 1,
				.pColorAttachments    = &(VkAttachmentReference){
					.attachment = 0,
					.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				},
				.pDepthStencilAttachment = &(VkAttachmentReference){
					.attachment = 1,
					.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				},
			},
			.dependencyCount = 1,
			.pDependencies   = &(VkSubpassDependency){
				.srcSubpass    = VK_SUBPASS_EXTERNAL,
				.dstSubpass    = 0,
				.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.srcAccessMask = 0,
				.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			},
		};
		VK_CHECK(vkCreateRenderPass(context->device, &render_pass_create_info, NULL, &render_graph.render_passes[1]));
	}
	return render_graph;
}
#endif

/*
GPU_Render_Graph GPU_Compile_Render_Pass(GPU_Context *context, xRender_Pass *render_pass) {
	VkSubpassDependency subpass_dependencies[render_pass->subpass_dependency_count];
	for (u32 i = 0; i < render_pass->subpass_dependency_count; i++) {
		subpass_dependencies[i] = render_pass->subpass_dependencies[i];
	}
	VkAttachmentDescription attachment_descriptions[render_pass->framebuffer_attachment_count];
	for (u32 i = 0; i < render_pass->framebuffer_attachment_count; i++) {
		attachment_descriptions[i] = render_pass->framebuffer_attachment_descriptions[i];
	}
	VkRenderPassCreateInfo render_pass_create_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = render_pass->framebuffer_attachment_count,
		.pAttachments = attachment_descriptions,
		.subpassCount = render_pass->subpass_count,
		.dependencyCount = render_pass->subpass_dependency_count,
		.pDependencies = subpass_dependencies,
	};
	VkSubpassDescription subpass_descriptions[render_pass->subpass_count];
	for (u32 i = 0; i < render_pass->subpass_count; i++) {
		subpass_descriptions[i] = render_pass->subpass_descriptions[i];
	}
	render_pass_create_info.pSubpasses = subpass_descriptions;
	GPU_Render_Pass gpu_render_pass;
	VK_CHECK(vkCreateRenderPass(vulkan_context.device, &render_pass_create_info, NULL, &gpu_render_pass));
	return gpu_render_pass;
}
*/


	//VkDescriptorPoolSize descriptor_pool_sizes[subpool_count];
	//for (u32 i = 0; i < subpool_count; i++) {
		//descriptor_pool_sizes[i].type = subpool_descriptions[i].type;
		//descriptor_pool_sizes[i].descriptorCount = subpool_descriptions[i].size;
	//}


VkDescriptorPool Render_API_Initialize_Descriptors(Render_API_Context *context, u32 swapchain_image_count) {
	//context->descriptor_set_count = GPU_NON_IMMEDIATE_DESCRIPTOR_COUNT + (GPU_IMMEDIATE_DESCRIPTOR_COUNT * swapchain_image_count);

	VkDescriptorPoolSize descriptor_pool_sizes[ArrayCount(vulkan_descriptor_pool_size_infos)];
	for (s32 i = 0; i < ArrayCount(vulkan_descriptor_pool_size_infos); i++) {
		descriptor_pool_sizes[i].type = vulkan_descriptor_pool_size_infos[i].type;
		descriptor_pool_sizes[i].descriptorCount = vulkan_descriptor_pool_size_infos[i].non_immediate_descriptor_count + (vulkan_descriptor_pool_size_infos[i].immediate_descriptor_count * swapchain_image_count) + 100; // @TODO
	}
	VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT,
		.maxSets = GPU_NON_IMMEDIATE_DESCRIPTOR_COUNT + (GPU_IMMEDIATE_DESCRIPTOR_COUNT * swapchain_image_count),
		.poolSizeCount = ArrayCount(descriptor_pool_sizes),
		.pPoolSizes = descriptor_pool_sizes,
	};
	VkDescriptorPool descriptor_pool;
	VK_CHECK(vkCreateDescriptorPool(context->device, &descriptor_pool_create_info, NULL, &descriptor_pool));
	Vulkan_Create_Descriptor_Set_Layouts(context, swapchain_image_count, context->descriptor_set_layouts);
	return descriptor_pool;
}

void Render_API_Create_Descriptor_Sets(Render_API_Context *context, GPU_Descriptor_Pool pool, GPU_Descriptor_Set_ID id, u32 set_count, GPU_Descriptor_Set *sets) {
	VkDescriptorSetLayout layouts[] = {
		context->descriptor_set_layouts[id],
		context->descriptor_set_layouts[id],
		context->descriptor_set_layouts[id],
	};
	Assert(ArrayCount(layouts) == set_count);
	VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = pool,
		.descriptorSetCount = set_count,
		.pSetLayouts = layouts,
	};
	VK_CHECK(vkAllocateDescriptorSets(context->device, &descriptor_set_allocate_info, sets));
}

void Render_API_Update_Descriptor_Sets(Render_API_Context *context, s32 swapchain_image_index, GPU_Descriptor_Set set, GPU_Buffer buffer) {
	VkDescriptorBufferInfo buffer_info = {
		.buffer = buffer,
		.offset = (u64)(swapchain_image_index * 0x100),
		.range = sizeof(M4),
	};
	VkWriteDescriptorSet descriptor_write = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = set,
		.dstBinding = 0,
		.dstArrayElement = 0,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.pBufferInfo = &buffer_info,
	};
	vkUpdateDescriptorSets(context->device, 1, &descriptor_write, 0, NULL);
}

#if 0
GPU_Descriptor_Set Render_API_Create_Shader_Descriptor_Sets(Render_API_Context *context, GPU_Shader_ID shader_id) {
	VkWriteDescriptorSet descriptor_writes[10]; // @TODO
	s32 write_count = 0;
	uniform_buffer = Create_GPU_Device_Buffer(context, sizeof(M4) * context->swapchain_image_count + 0x100 * context->swapchain_image_count, GPU_UNIFORM_BUFFER | GPU_TRANSFER_DESTINATION_BUFFER);
	VkDescriptorBufferInfo buffer_infos[context->swapchain_image_count];
	s32 buffer_info_count = 0;
	for (s32 i = 0; i < context->swapchain_image_count; i++) {
		buffer_infos[buffer_info_count] = (VkDescriptorBufferInfo){
			.buffer = uniform_buffer,
			.offset = i * 0x100,
			.range  = sizeof(M4),
		};
		descriptor_writes[write_count++] = (VkWriteDescriptorSet){
			.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet          = context->descriptor_sets.rusted_iron_vertex_bind_per_object_update_immediate[i],
			.dstBinding      = 0,
			.dstArrayElement = 0,
			.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.pBufferInfo     = &buffer_infos[buffer_info_count],
		};
		buffer_info_count++;
	}
	vkUpdateDescriptorSets(context->api_context.device, write_count, descriptor_writes, 0, NULL);
}
#endif

typedef struct GPU_Image_Descriptor_Write {
	GPU_Image_Layout layout;
	GPU_Image_View view;
	GPU_Sampler sampler;
} GPU_Image_Descriptor_Write;

typedef struct GPU_Buffer_Descriptor_Write {
	GPU_Buffer buffer;
	u32 offset;
	u32 range;
} GPU_Buffer_Descriptor_Write;

typedef struct GPU_Descriptor_Write {
	GPU_Image_Descriptor_Write *image;
	GPU_Buffer_Descriptor_Write *buffer;
} GPU_Descriptor_Write;

typedef struct GPU_Descriptor_Description {
	u32 count;
	u32 binding;
	GPU_Descriptor_Type type;
	GPU_Shader_Stage stage;
	GPU_Descriptor_Write write; 
	u32 flags;
} GPU_Descriptor_Description;

#if 0
void GPU_Create_Descriptor_Sets(Render_API_Context *context, s32 count, GPU_Descriptor_Pool descriptor_pool, GPU_Descriptor_Set *sets) {
	VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = descriptor_pool,
		.descriptorSetCount = count,
		.pSetLayouts = &descriptor_set.layout,
	};
	VK_CHECK(vkAllocateDescriptorSets(context->device, &descriptor_set_allocate_info, &descriptor_set.descriptor_set));
}

GPU_Descriptor_Set GPU_Create_Descriptor_Set(Render_API_Context *context, u32 descriptor_count, GPU_Descriptor_Description *descriptor_descriptions, u32 flags, VkDescriptorPool descriptor_pool) {
	GPU_Descriptor_Set descriptor_set;
	VkDescriptorSetLayoutBinding bindings[descriptor_count];
	for (u32 i = 0; i < descriptor_count; i++) {
		bindings[i] = (VkDescriptorSetLayoutBinding){
			.binding = descriptor_descriptions[i].binding,
			.descriptorType = descriptor_descriptions[i].type,
			.descriptorCount = descriptor_descriptions[i].count,
			.stageFlags = descriptor_descriptions[i].stage_flags,
			.pImmutableSamplers = NULL,
		};
	}
	VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = descriptor_count,
		.pBindings = bindings,
		.flags = flags,
	};
	VK_CHECK(vkCreateDescriptorSetLayout(context->device, &descriptor_set_layout_create_info, NULL, &descriptor_set.layout));
	VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = descriptor_pool,
		.descriptorSetCount = 1,
		.pSetLayouts = &descriptor_set.layout,
	};
	VK_CHECK(vkAllocateDescriptorSets(context->device, &descriptor_set_allocate_info, &descriptor_set.descriptor_set));

	VkWriteDescriptorSet descriptor_writes[descriptor_count];
	VkDescriptorImageInfo descriptor_image_infos[descriptor_count];
	VkDescriptorBufferInfo descriptor_buffer_infos[descriptor_count];
	for (u32 i = 0; i < descriptor_count; i++) {
		descriptor_writes[i] = (VkWriteDescriptorSet){
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = descriptor_set.descriptor_set,
			.dstBinding = descriptor_descriptions[i].binding,
			.dstArrayElement = 0,
			.descriptorType = descriptor_descriptions[i].type,
			.descriptorCount = 1,
		};
		Assert(descriptor_descriptions[i].write.image || descriptor_descriptions[i].write.buffer);
		if (descriptor_descriptions[i].write.image) {
			 descriptor_image_infos[i] = (VkDescriptorImageInfo){
				.imageLayout = descriptor_descriptions[i].write.image->layout,
				.imageView = descriptor_descriptions[i].write.image->view,
				.sampler = descriptor_descriptions[i].write.image->sampler,
			};
			descriptor_writes[i].pImageInfo = &descriptor_image_infos[i];
		} else {
			descriptor_buffer_infos[i] = (VkDescriptorBufferInfo){
				.buffer = descriptor_descriptions[i].write.buffer->buffer,
				.offset = descriptor_descriptions[i].write.buffer->offset,
				.range = descriptor_descriptions[i].write.buffer->range,
			};
			descriptor_writes[i].pBufferInfo = &descriptor_buffer_infos[i];
		}
	}
	vkUpdateDescriptorSets(context->device, descriptor_count, descriptor_writes, 0, NULL); // No return.
	return descriptor_set;
}
#endif

typedef struct Push_Constant_Description {
	u32 offset;
	u32 size;
	GPU_Shader_Stage shader_stage;
} Push_Constant_Description;

typedef struct GPU_Framebuffer_Attachment_Color_Blend_Description {
	bool enable_blend;
	GPU_Blend_Factor source_color_blend_factor;
	GPU_Blend_Factor destination_color_blend_factor;
	GPU_Blend_Operation color_blend_operation;
	GPU_Blend_Factor source_alpha_blend_factor;
	GPU_Blend_Factor destination_alpha_blend_factor;
	GPU_Blend_Operation alpha_blend_operation;
	GPU_Color_Component_Flags color_write_mask;
} GPU_Framebuffer_Attachment_Color_Blend_Description;

typedef struct GPU_Pipeline_Vertex_Input_Attribute_Description {
	GPU_Format format;
	u32 binding;
	u32 location;
	u32 offset;
} GPU_Pipeline_Vertex_Input_Attribute_Description;

typedef struct GPU_Pipeline_Vertex_Input_Binding_Description {
	u32 binding;
	u32 stride;
	GPU_Vertex_Input_Rate input_rate;
} GPU_Pipeline_Vertex_Input_Binding_Description;

/*
typedef struct GPU_Shader_Stage_Description {
	 GPU_Shader_Stage_Flags stage_flags;
	 GPU_Shader_Module module;
} GPU_Shader_Stage_Description;
*/

typedef struct GPU_Pipeline_Description {
	u32 descriptor_set_layout_count;
	GPU_Descriptor_Set_Layout *descriptor_set_layouts;
	u32 push_constant_count;
	Push_Constant_Description *push_constant_descriptions;
	GPU_Pipeline_Topology topology;
	f32 viewport_width;
	f32 viewport_height;
	u32 scissor_width;
	u32 scissor_height;
	GPU_Compare_Operation depth_compare_operation;
	u32 framebuffer_attachment_color_blend_count;
	GPU_Framebuffer_Attachment_Color_Blend_Description *framebuffer_attachment_color_blend_descriptions;
	u32 vertex_input_attribute_count;
	GPU_Pipeline_Vertex_Input_Attribute_Description *vertex_input_attribute_descriptions;
	u32 vertex_input_binding_count;
	GPU_Pipeline_Vertex_Input_Binding_Description *vertex_input_binding_descriptions;
	u32 dynamic_state_count;
	GPU_Dynamic_Pipeline_State *dynamic_states;
	GPU_Shader shader;
	GPU_Render_Pass render_pass;
	bool enable_depth_bias;
} GPU_Pipeline_Description;

GPU_Pipeline Render_API_Create_Pipeline(Render_API_Context *context, GPU_Pipeline_Description *pipeline_description) {
	GPU_Pipeline pipeline;
	VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = pipeline_description->descriptor_set_layout_count,
		.pSetLayouts = pipeline_description->descriptor_set_layouts,
		.pushConstantRangeCount = pipeline_description->push_constant_count,
	};
	VkPushConstantRange push_constant_ranges[pipeline_description->push_constant_count];
	for (u32 i = 0; i < pipeline_description->push_constant_count; i++) {
		push_constant_ranges[i] = (VkPushConstantRange){
			.stageFlags = pipeline_description->push_constant_descriptions[i].shader_stage,
			.offset = pipeline_description->push_constant_descriptions[i].offset,
			.size = pipeline_description->push_constant_descriptions[i].offset,
		};
	}
	pipeline_layout_create_info.pPushConstantRanges = push_constant_ranges;
	VK_CHECK(vkCreatePipelineLayout(context->device, &pipeline_layout_create_info, NULL, &pipeline.layout));

	VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = (VkPrimitiveTopology)pipeline_description->topology,
		.primitiveRestartEnable = VK_FALSE,
	};
	VkViewport viewport = {
		.x = 0.0f,
		.y = 0.0f,
		.width = pipeline_description->viewport_width,
		.height = pipeline_description->viewport_height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};
	VkRect2D scissor = {
		.offset = {0, 0},
		.extent = (VkExtent2D){pipeline_description->scissor_width, pipeline_description->scissor_height},
	};
	VkPipelineViewportStateCreateInfo viewport_state_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor,
	};
	VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.depthBiasEnable = pipeline_description->enable_depth_bias,
		.lineWidth = 1.0f,
	};
	VkPipelineMultisampleStateCreateInfo multisample_state_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE,
	};
	VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
		.depthCompareOp = (VkCompareOp)pipeline_description->depth_compare_operation,
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = VK_FALSE,
	};
	VkPipelineColorBlendAttachmentState color_blend_attachment_states[pipeline_description->framebuffer_attachment_color_blend_count];
	for (u32 i = 0; i < pipeline_description->framebuffer_attachment_color_blend_count; i++) {
		color_blend_attachment_states[i] = (VkPipelineColorBlendAttachmentState){
			.blendEnable = pipeline_description->framebuffer_attachment_color_blend_descriptions[i].enable_blend,
			.srcColorBlendFactor = (VkBlendFactor)pipeline_description->framebuffer_attachment_color_blend_descriptions[i].source_color_blend_factor,
			.dstColorBlendFactor = (VkBlendFactor)pipeline_description->framebuffer_attachment_color_blend_descriptions[i].destination_color_blend_factor,
			.colorBlendOp = (VkBlendOp)pipeline_description->framebuffer_attachment_color_blend_descriptions[i].color_blend_operation,
			.srcAlphaBlendFactor = (VkBlendFactor)pipeline_description->framebuffer_attachment_color_blend_descriptions[i].source_alpha_blend_factor,
			.dstAlphaBlendFactor = (VkBlendFactor)pipeline_description->framebuffer_attachment_color_blend_descriptions[i].destination_alpha_blend_factor,
			.alphaBlendOp = (VkBlendOp)pipeline_description->framebuffer_attachment_color_blend_descriptions[i].alpha_blend_operation,
			.colorWriteMask = pipeline_description->framebuffer_attachment_color_blend_descriptions[i].color_write_mask,
		};
	}
	VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = pipeline_description->framebuffer_attachment_color_blend_count,
		.pAttachments = color_blend_attachment_states,
		.blendConstants = {},
	};
	VkVertexInputAttributeDescription vertex_input_attribute_descriptions[pipeline_description->vertex_input_attribute_count];
	for (u32 i = 0; i < pipeline_description->vertex_input_attribute_count; i++) {
		vertex_input_attribute_descriptions[i] = (VkVertexInputAttributeDescription){
			.location = pipeline_description->vertex_input_attribute_descriptions[i].location,
			.binding = pipeline_description->vertex_input_attribute_descriptions[i].binding,
			.format = (VkFormat)pipeline_description->vertex_input_attribute_descriptions[i].format,
			.offset = pipeline_description->vertex_input_attribute_descriptions[i].offset,
		};
	}
	VkVertexInputBindingDescription vertex_input_binding_descriptions[pipeline_description->vertex_input_binding_count];
	for (u32 i = 0; i < pipeline_description->vertex_input_binding_count; i++) {
		vertex_input_binding_descriptions[i] = (VkVertexInputBindingDescription){
			.binding = pipeline_description->vertex_input_binding_descriptions[i].binding,
			.stride = pipeline_description->vertex_input_binding_descriptions[i].stride,
			.inputRate = (VkVertexInputRate)pipeline_description->vertex_input_binding_descriptions[i].input_rate,
		};
	}
	VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = pipeline_description->vertex_input_binding_count,
		.pVertexBindingDescriptions = vertex_input_binding_descriptions,
		.vertexAttributeDescriptionCount = pipeline_description->vertex_input_attribute_count,
		.pVertexAttributeDescriptions = vertex_input_attribute_descriptions,
	};
	VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = pipeline_description->dynamic_state_count,
		.pDynamicStates = (VkDynamicState *)pipeline_description->dynamic_states,
	};
	VkPipelineShaderStageCreateInfo shader_stage_create_infos[pipeline_description->shader.stage_count];
	for (s32 i = 0; i < pipeline_description->shader.stage_count; i++) {
		shader_stage_create_infos[i] = (VkPipelineShaderStageCreateInfo){
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = (VkShaderStageFlagBits)pipeline_description->shader.stages[i],
			.module = pipeline_description->shader.modules[i],
			.pName = "main",
		};
	}
	VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = (u32)pipeline_description->shader.stage_count,
		.pStages = shader_stage_create_infos,
		.pVertexInputState = &vertex_input_state_create_info,
		.pInputAssemblyState = &input_assembly_create_info,
		.pViewportState = &viewport_state_create_info,
		.pRasterizationState = &rasterization_state_create_info,
		.pMultisampleState = &multisample_state_create_info,
		.pDepthStencilState = &depth_stencil_state_create_info,
		.pColorBlendState = &color_blend_state_create_info,
		.pDynamicState = &dynamic_state_create_info,
		.layout = pipeline.layout,
		.renderPass = pipeline_description->render_pass,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1,
	};
	VK_CHECK(vkCreateGraphicsPipelines(context->device, NULL, 1, &graphics_pipeline_create_info, NULL, &pipeline.pipeline));
	return pipeline;
}

VkFramebuffer Render_API_Create_Framebuffer(Render_API_Context *context, VkRenderPass render_pass, u32 width, u32 height, u32 attachment_count, VkImageView *attachments) {
	VkFramebufferCreateInfo framebuffer_create_info = {
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.renderPass = render_pass,
		.attachmentCount = attachment_count,
		.pAttachments = attachments,
		.width = width,
		.height = height,
		.layers = 1,
	};
	VkFramebuffer framebuffer;
	VK_CHECK(vkCreateFramebuffer(context->device, &framebuffer_create_info, NULL, &framebuffer));
	return framebuffer;
}

VkMemoryRequirements Render_API_Get_Image_Allocation_Requirements(GPU_Image image) {
	VkMemoryRequirements imageMemoryRequirements;
	vkGetImageMemoryRequirements(vulkanContext.device, image, &imageMemoryRequirements);
	return imageMemoryRequirements;
}

VkImage Render_API_Create_Image(u32 width, u32 height, GPU_Format format, GPU_Image_Layout initialLayout, GPU_Image_Usage_Flags usageFlags, GPU_Sample_Count_Flags sampleCountFlags) {
	VkImageCreateInfo imageCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = (VkFormat)format,
		.extent = {
			.width = width,
			.height = height,
			.depth = 1,
		},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = (VkSampleCountFlagBits)sampleCountFlags,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = usageFlags,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = (VkImageLayout)initialLayout,
	};
	GPU_Image image;
	VK_CHECK(vkCreateImage(vulkanContext.device, &imageCreateInfo, NULL, &image));
	return image;
}

void Render_API_Bind_Image_Memory(VkImage image, VkDeviceMemory memory, u32 offset) {
	VK_CHECK(vkBindImageMemory(vulkanContext.device, image, memory, offset));
}

VkImageView Render_API_Create_Image_View(Render_API_Context *context, VkImage image, VkFormat format, VkImageUsageFlags usage_flags) {
	VkImageViewCreateInfo image_view_create_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = format,
		.components = {
			.r = VK_COMPONENT_SWIZZLE_IDENTITY,
			.g = VK_COMPONENT_SWIZZLE_IDENTITY,
			.b = VK_COMPONENT_SWIZZLE_IDENTITY,
			.a = VK_COMPONENT_SWIZZLE_IDENTITY,
		},
		.subresourceRange = {
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
	};
	if (usage_flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
		image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	} else {
		image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}
	VkImageView image_view;
	VK_CHECK(vkCreateImageView(context->device, &image_view_create_info, NULL, &image_view));
	return image_view;
}

void Render_API_Transition_Image_Layout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
	VkImageMemoryBarrier barrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.oldLayout = oldLayout,
		.newLayout = newLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = image,
		.subresourceRange = {
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
	};
	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	} else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}
	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;
	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	} else {
		Abort("unsupported Vulkan image layout transition");
	}
	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, NULL, 0, NULL, 1, &barrier); // No return.
}

void Render_API_Record_Copy_Buffer_To_Image_Command(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, u32 imageWidth, u32 imageHeight) {
	VkBufferImageCopy region = {
		.bufferOffset = 0,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,
		.imageSubresource = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
		.imageOffset = {},
		.imageExtent = {
			imageWidth,
			imageHeight,
			1,
		},
	};
	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void Render_API_Present_Swapchain_Image(Render_API_Context *context, GPU_Swapchain swapchain, u32 swapchain_image_index, u32 currentFrame /* @TODO */) {
	VkSemaphore wait_semaphores[] = {context->image_available_semaphores[currentFrame]};
	VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSemaphore signal_semaphores[] = {context->render_finished_semaphores[currentFrame]};
	VkSwapchainKHR swapchains[] = {swapchain};
	VkPresentInfoKHR present_info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = ArrayCount(signal_semaphores),
		.pWaitSemaphores = signal_semaphores,
		.swapchainCount = ArrayCount(swapchains),
		.pSwapchains = swapchains,
		.pImageIndices = &swapchain_image_index,
	};
	VK_CHECK(vkQueuePresentKHR(context->present_queue, &present_info));
}

#if 0
GPU_Image GPU_Create_Image(GPU_Context *context, Render_Image_Creation_Parameters *parameters, GPU_Memory memory, u32 offset) {
	GPU_Image image = {
		.format = parameters->format,
		.layout = parameters->initial_layout,
	};
	image.image = Vulkan_Create_Image(context, parameters);
	// Bind the image to GPU memory.
	{
		// @TODO: Should this check be debug only?
		// Check that this image type is compatible with this memory type.
		VkMemoryRequirements image_memory_requirements;
		vkGetImageMemoryRequirements(context->vulkan.device, image.image, &image_memory_requirements);
		VkPhysicalDeviceMemoryProperties physical_device_memory_properties; // @TODO: Store these properties?
		vkGetPhysicalDeviceMemoryProperties(context->vulkan.physical_device, &physical_device_memory_properties);
		/* @TODO: How to get memory_type?
		s32 selected_memory_type_index = -1;
		for (u32 i = 0; i < physical_device_memory_properties.memoryTypeCount; i++) {
			if ((image_memory_requirements.memoryTypeBits & (1 << i)) && ((physical_device_memory_properties.memoryTypes[i].propertyFlags & memory_type) == memory_type)) {
				selected_memory_type_index = i;
				break;
			}
		}
		Assert(selected_memory_type_index != -1);
		*/
		VK_CHECK(vkBindImageMemory(context->vulkan.device, image.image, memory, offset));
	}
	// Create an image view.
	{
		VkImageViewCreateInfo image_view_create_info = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = image.image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = parameters->format,
			.components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
			.components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
			.components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
			.components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
			.subresourceRange.baseMipLevel = 0,
			.subresourceRange.levelCount = 1,
			.subresourceRange.baseArrayLayer = 0,
			.subresourceRange.layerCount = 1,
		};
		if (parameters->usage_flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
			image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		} else {
			image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}
		VK_CHECK(vkCreateImageView(context->vulkan.device, &image_view_create_info, NULL, &image.view));
	}
	return image;
}
#endif

void _Vulkan_Transition_Image_Layout(VkCommandBuffer command_buffer, VkImage image, VkImageLayout old_layout, VkImageLayout new_layout, VkFormat format) {
	VkImageMemoryBarrier barrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.oldLayout = old_layout,
		.newLayout = new_layout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = image,
		.subresourceRange = {
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
	};
	if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	} else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}
	VkPipelineStageFlags source_stage;
	VkPipelineStageFlags destination_stage;
	if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	} else {
		Abort("Unsupported Vulkan layout transition");
	}
	vkCmdPipelineBarrier(command_buffer, source_stage, destination_stage, 0, 0, NULL, 0, NULL, 1, &barrier);
}

VkRenderPass TEMPORARY_Render_API_Create_Render_Pass(Render_API_Context *context) {
	VkAttachmentDescription attachments[] = {
		{
			.format = context->window_surface_format.format,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		},
		{
			.format = VK_FORMAT_D32_SFLOAT_S8_UINT,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		},
	};
	VkAttachmentReference ColorAttachment = {
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};
	VkAttachmentReference StencilAttachment = {
		.attachment = 1,
		.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};
	VkSubpassDescription SubpassDescription = {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &ColorAttachment,
		.pDepthStencilAttachment = &StencilAttachment,
	};
	VkSubpassDependency SubpassDependency = {
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
	};
	VkRenderPassCreateInfo render_pass_create_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = ArrayCount(attachments),
		.pAttachments = attachments,
		.subpassCount = 1,
		.pSubpasses = &SubpassDescription,
		.dependencyCount = 1,
		.pDependencies = &SubpassDependency,
	};
	VkRenderPass render_pass;
	VK_CHECK(vkCreateRenderPass(context->device, &render_pass_create_info, NULL, &render_pass));
	return render_pass;
}

typedef struct GPU_Sampler_Description {
	GPU_Sampler_Filter filter;
	GPU_Sampler_Address_Mode address_mode_u;
	GPU_Sampler_Address_Mode address_mode_v;
	GPU_Sampler_Address_Mode address_mode_w;
	f32 mipmap_lod_bias;
	u32 max_anisotropy;
	f32 min_lod;
	f32 max_lod;
	GPU_Border_Color border_color;
} GPU_Sampler_Description;

VkSampler Vulkan_Create_Sampler(Render_API_Context *context, VkSamplerCreateInfo *sampler_create_info) {
	VkSampler sampler;
	VK_CHECK(vkCreateSampler(context->device, sampler_create_info, NULL, &sampler));
	return sampler;
#if 0
	VkSamplerCreateInfo sampler_create_info = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = sampler_description.filter.vulkan.mag,
		.minFilter = sampler_description.filter.vulkan.min,
		.mipmapMode = sampler_description.filter.vulkan.mipmap,
		.addressModeU = sampler_description.address_mode_u,
		.addressModeV = sampler_description.address_mode_v,
		.addressModeW = sampler_description.address_mode_w,
		.mipLodBias = sampler_description.mipmap_lod_bias,
		.maxAnisotropy = sampler_description.max_anisotropy,
		.minLod = sampler_description.min_lod,
		.maxLod = sampler_description.max_lod,
		.borderColor = sampler_description.border_color,
	};
	VkSampler sampler;
	VK_CHECK(vkCreateSampler(context->vulkan.device, &sampler_create_info, NULL, &sampler));
	return sampler;
#endif
}

#if 0

void create_vulkan_display_objects(Memory_Arena *arena) {
	// Create swapchain.
	{
		VkSurfaceCapabilitiesKHR surface_capabilities;
		VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkan_context.physical_device, vulkan_context.surface, &surface_capabilities));
		if (surface_capabilities.currentExtent.width != UINT32_MAX) {
			vulkan_context.swapchain_image_extent = surface_capabilities.currentExtent;
		} else {
			vulkan_context.swapchain_image_extent.width  = u32_max(surface_capabilities.minImageExtent.width, u32_min(surface_capabilities.maxImageExtent.width, window_width));
			vulkan_context.swapchain_image_extent.height = u32_max(surface_capabilities.minImageExtent.height, u32_min(surface_capabilities.maxImageExtent.height, window_height));
		}
		u32 num_requested_swapchain_images = surface_capabilities.minImageCount + 1;
		if (surface_capabilities.maxImageCount > 0 && num_requested_swapchain_images > surface_capabilities.maxImageCount) {
			num_requested_swapchain_images = surface_capabilities.maxImageCount;
		}
		VkSwapchainCreateInfoKHR swapchain_create_info = {
			.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface          = vulkan_context.surface,
			.minImageCount    = num_requested_swapchain_images,
			.imageFormat      = vulkan_context.surface_format.format,
			.imageColorSpace  = vulkan_context.surface_format.colorSpace,
			.imageExtent      = vulkan_context.swapchain_image_extent,
			.imageArrayLayers = 1,
			.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.preTransform     = surface_capabilities.currentTransform,
			.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode      = vulkan_context.present_mode,
			.clipped          = 1,
			.oldSwapchain     = NULL,
		};
		if (vulkan_context.graphics_queue_family != vulkan_context.present_queue_family) {
			u32 queue_family_indices[] = { vulkan_context.graphics_queue_family, vulkan_context.present_queue_family };
			swapchain_create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
			swapchain_create_info.queueFamilyIndexCount = ArrayCount(queue_family_indices);
			swapchain_create_info.pQueueFamilyIndices   = queue_family_indices;
		} else {
			swapchain_create_info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
		}
		VK_CHECK(vkCreateSwapchainKHR(vulkan_context.device, &swapchain_create_info, NULL, &vulkan_context.swapchain));
	}

	// Swapchain images.
	{
		VK_CHECK(vkGetSwapchainImagesKHR(vulkan_context.device, vulkan_context.swapchain, &vulkan_context.num_swapchain_images, NULL));
		VkImage *swapchain_images = allocate_array(arena, VkImage, vulkan_context.num_swapchain_images);
		VK_CHECK(vkGetSwapchainImagesKHR(vulkan_context.device, vulkan_context.swapchain, &vulkan_context.num_swapchain_images, swapchain_images));
		for (s32 i = 0; i < vulkan_context.num_swapchain_images; i++) {
			VkImageViewCreateInfo image_view_create_info = {
				.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image                           = swapchain_images[i],
				.viewType                        = VK_IMAGE_VIEW_TYPE_2D,
				.format                          = vulkan_context.surface_format.format,
				.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY,
				.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY,
				.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY,
				.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY,
				.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
				.subresourceRange.baseMipLevel   = 0,
				.subresourceRange.levelCount     = 1,
				.subresourceRange.baseArrayLayer = 0,
				.subresourceRange.layerCount     = 1,
			};
			VK_CHECK(vkCreateImageView(vulkan_context.device, &image_view_create_info, NULL, &vulkan_context.swapchain_image_views[i]));
		}
	}

	// Render pass.
	{
		VkSubpassDependency subpass_dependencies[] = {
			{
				.srcSubpass      = VK_SUBPASS_EXTERNAL,
				.dstSubpass      = 0,
				.srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				.dstStageMask    = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
				.srcAccessMask   = VK_ACCESS_SHADER_READ_BIT,
				.dstAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
			},
			{
				.srcSubpass      = 0,
				.dstSubpass      = VK_SUBPASS_EXTERNAL,
				.srcStageMask    = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				.dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				.srcAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				.dstAccessMask   = VK_ACCESS_SHADER_READ_BIT,
				.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
			}
		};
		VkRenderPassCreateInfo render_pass_create_info = {
			.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = 1,
			.pAttachments    = &(VkAttachmentDescription){
				.format         = SHADOW_MAP_DEPTH_FORMAT,
				.samples        = VK_SAMPLE_COUNT_1_BIT,
				.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
			},
			.subpassCount    = 1,
			.pSubpasses      = &(VkSubpassDescription){
				.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.colorAttachmentCount    = 0,
				.pDepthStencilAttachment = &(VkAttachmentReference){
					.attachment = 0,
					.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				},
			},
			.dependencyCount = ArrayCount(subpass_dependencies),
			.pDependencies   = subpass_dependencies,
		};
		VK_CHECK(vkCreateRenderPass(vulkan_context.device, &render_pass_create_info, NULL, &vulkan_context.render_passes.shadow_map));
	}

	{
		VkAttachmentDescription attachments[] = {
			{
				.format         = vulkan_context.surface_format.format,
				.samples        = VK_SAMPLE_COUNT_1_BIT,
				.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			},
			{
				.format         = VK_FORMAT_D32_SFLOAT_S8_UINT,
				.samples        = VK_SAMPLE_COUNT_1_BIT,
				.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			},
		};
		VkRenderPassCreateInfo render_pass_create_info = {
			.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = ArrayCount(attachments),
			.pAttachments    = attachments,
			.subpassCount    = 1,
			.pSubpasses      = &(VkSubpassDescription){
				.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.colorAttachmentCount = 1,
				.pColorAttachments    = &(VkAttachmentReference){
					.attachment = 0,
					.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				},
				.pDepthStencilAttachment = &(VkAttachmentReference){
					.attachment = 1,
					.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				},
			},
			.dependencyCount = 1,
			.pDependencies   = &(VkSubpassDependency){
				.srcSubpass    = VK_SUBPASS_EXTERNAL,
				.dstSubpass    = 0,
				.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.srcAccessMask = 0,
				.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			},
		};
		VK_CHECK(vkCreateRenderPass(vulkan_context.device, &render_pass_create_info, NULL, &vulkan_context.render_pass));
		//attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		//attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		//VK_CHECK(vkCreateRenderPass(vulkan_context.device, &render_pass_create_info, NULL, &vulkan_context.render_passes.flat_color));
	}

	// Pipeline layouts.
	{
		{
			VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
				.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
				.setLayoutCount         = ArrayCount(vulkan_context.descriptor_set_layouts.scene),
				.pSetLayouts            = vulkan_context.descriptor_set_layouts.scene,
				.pushConstantRangeCount = 1,
				.pPushConstantRanges    = &(VkPushConstantRange){
					.offset = 0,
					.size = sizeof(Push_Constants),
					.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
				},
			};
			VK_CHECK(vkCreatePipelineLayout(vulkan_context.device, &pipeline_layout_create_info, NULL, &vulkan_context.pipeline_layout));
		}
		{
			VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
				.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
				.setLayoutCount         = ArrayCount(vulkan_context.descriptor_set_layouts.shadow_map),
				.pSetLayouts            = vulkan_context.descriptor_set_layouts.shadow_map,
			};
			VK_CHECK(vkCreatePipelineLayout(vulkan_context.device, &pipeline_layout_create_info, NULL, &vulkan_context.pipeline_layouts.shadow_map));
		}
		{
			VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
				.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
				.setLayoutCount         = ArrayCount(vulkan_context.descriptor_set_layouts.flat_color),
				.pSetLayouts            = vulkan_context.descriptor_set_layouts.flat_color,
				.pushConstantRangeCount = 1,
				.pPushConstantRanges    = &(VkPushConstantRange){
					.offset = 0,
					.size = sizeof(V4),
					.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
				},
			};
			VK_CHECK(vkCreatePipelineLayout(vulkan_context.device, &pipeline_layout_create_info, NULL, &vulkan_context.pipeline_layouts.flat_color));
		}
	}

	// Pipelines.
	{
		VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info = {
			.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = VK_FALSE,
		};
		VkViewport viewport = {
			.x        = 0.0f,
			.y        = 0.0f,
			.width    = (f32)vulkan_context.swapchain_image_extent.width,
			.height   = (f32)vulkan_context.swapchain_image_extent.height,
			.minDepth = 0.0f,
			.maxDepth = 1.0f,
		};
		VkRect2D scissor = {
			.offset = {0, 0},
			.extent = vulkan_context.swapchain_image_extent,
		};
		VkPipelineViewportStateCreateInfo viewport_state_create_info = {
			.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1,
			.pViewports    = &viewport,
			.scissorCount  = 1,
			.pScissors     = &scissor,
		};
		VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = {
			.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.depthClampEnable        = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode             = VK_POLYGON_MODE_FILL,
			.lineWidth               = 1.0f,
			.cullMode                = VK_CULL_MODE_BACK_BIT,
			.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable         = VK_FALSE,
		};
		VkPipelineMultisampleStateCreateInfo multisample_state_create_info = {
			.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.sampleShadingEnable   = VK_FALSE,
			.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
		};
		VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info = {
			.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthTestEnable       = VK_TRUE,
			.depthWriteEnable      = VK_TRUE,
			.depthCompareOp        = VK_COMPARE_OP_LESS,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable     = VK_FALSE,
		};
		VkPipelineColorBlendAttachmentState color_blend_attachment_state = {
			.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			.blendEnable         = VK_TRUE,
			.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
			.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			//.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
			//.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
			.colorBlendOp        = VK_BLEND_OP_ADD,
			.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
			.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
			.alphaBlendOp        = VK_BLEND_OP_ADD,
		};
		VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {
			.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable     = VK_FALSE,
			.logicOp           = VK_LOGIC_OP_COPY,
			.attachmentCount   = 1,
			.pAttachments      = &color_blend_attachment_state,
			.blendConstants[0] = 0.0f,
			.blendConstants[1] = 0.0f,
			.blendConstants[2] = 0.0f,
			.blendConstants[3] = 0.0f,
		};

		VkGraphicsPipelineCreateInfo requests[20]; // @TODO
		VkPipeline *results[20]; // @TODO
		u32 request_count = 0, result_count = 0;

		// PBR 
		{
			VkVertexInputAttributeDescription vertex_input_attribute_descriptions[] = {
				{
					.binding  = VULKAN_VERTEX_BUFFER_BIND_ID,
					.location = 0,
					.format   = VK_FORMAT_R32G32B32_SFLOAT,
					.offset   = offsetof(Vertex, position),
				},
				{
					.binding  = VULKAN_VERTEX_BUFFER_BIND_ID,
					.location = 1,
					.format   = VK_FORMAT_R32G32B32_SFLOAT,
					.offset   = offsetof(Vertex, color),
				},
				{
					.binding  = VULKAN_VERTEX_BUFFER_BIND_ID,
					.location = 2,
					.format   = VK_FORMAT_R32G32_SFLOAT,
					.offset   = offsetof(Vertex, uv),
				},
				{
					.binding  = VULKAN_VERTEX_BUFFER_BIND_ID,
					.location = 3,
					.format   = VK_FORMAT_R32G32B32_SFLOAT,
					.offset   = offsetof(Vertex, normal),
				},
				// @TODO: Load the bitangents from Assimp rather than calculating them in the shader.
				{
					.binding  = VULKAN_VERTEX_BUFFER_BIND_ID,
					.location = 4,
					.format   = VK_FORMAT_R32G32B32_SFLOAT,
					.offset   = offsetof(Vertex, tangent),
				},
				{
					.binding  = VULKAN_INSTANCE_BUFFER_BIND_ID,
					.location = 5,
					.format   = VK_FORMAT_R32_UINT,
					.offset   = offsetof(Instance_Data, material_id),
				},
			};
			VkVertexInputBindingDescription vertex_input_binding_descriptions[] = {
					{
						.binding   = VULKAN_VERTEX_BUFFER_BIND_ID,
						.stride    = sizeof(Vertex),
						.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
					},
					{
						.binding   = VULKAN_INSTANCE_BUFFER_BIND_ID,
						.stride    = sizeof(Instance_Data),
						.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
					},
			};
			// @TODO: Fix shadow map vertex input attributes (only needs position).
			VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {
				.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
				.vertexBindingDescriptionCount   = ArrayCount(vertex_input_binding_descriptions),
				.pVertexBindingDescriptions      = vertex_input_binding_descriptions,
				.vertexAttributeDescriptionCount = ArrayCount(vertex_input_attribute_descriptions),
				.pVertexAttributeDescriptions    = vertex_input_attribute_descriptions,
			};
			VkDynamicState dynamic_states[] = {
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_SCISSOR,
			};
			VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {
				.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
				.dynamicStateCount = ArrayCount(dynamic_states),
				.pDynamicStates    = dynamic_states,
			};
			//VkPipelineShaderStageCreateInfo *shader_stage_create_infos = allocate_array(arena, VkPipelineShaderStageCreateInfo, requests[i].shaders->module_count);
			VkPipelineShaderStageCreateInfo shader_stage_create_infos[MAX_SHADER_MODULES] = {};
			for (s32 j = 0; j < vulkan_context.shaders[TEXTURED_STATIC_SHADER].module_count; j++) {
				shader_stage_create_infos[j].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				shader_stage_create_infos[j].stage  = vulkan_context.shaders[TEXTURED_STATIC_SHADER].modules[j].stage;
				shader_stage_create_infos[j].module = vulkan_context.shaders[TEXTURED_STATIC_SHADER].modules[j].module;
				shader_stage_create_infos[j].pName  = "main";
			}
			VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {
				.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
				.stageCount          = vulkan_context.shaders[TEXTURED_STATIC_SHADER].module_count,
				.pStages             = shader_stage_create_infos,
				.pVertexInputState   = &vertex_input_state_create_info,
				.pInputAssemblyState = &input_assembly_create_info,
				.pViewportState      = &viewport_state_create_info,
				.pRasterizationState = &rasterization_state_create_info,
				.pMultisampleState   = &multisample_state_create_info,
				.pColorBlendState    = &color_blend_state_create_info,
				.pDepthStencilState  = &depth_stencil_state_create_info,
				.pDynamicState       = &dynamic_state_create_info,
				.layout              = vulkan_context.pipeline_layout,
				.renderPass          = vulkan_context.render_pass,
				.subpass             = 0,
				.basePipelineIndex   = -1,
				.basePipelineHandle  = VK_NULL_HANDLE,
			};
			VK_CHECK(vkCreateGraphicsPipelines(vulkan_context.device, NULL, 1, &graphics_pipeline_create_info, NULL, &vulkan_context.pipelines.textured_static));
		}

		// Shadow Map
		{
			VkVertexInputAttributeDescription vertex_input_attribute_descriptions[] = {
				{
					.binding  = VULKAN_VERTEX_BUFFER_BIND_ID,
					.location = 0,
					.format   = VK_FORMAT_R32G32B32_SFLOAT,
					.offset   = offsetof(Vertex, position),
				},
			};
			VkVertexInputBindingDescription vertex_input_binding_descriptions[] = {
					{
						.binding   = VULKAN_VERTEX_BUFFER_BIND_ID,
						.stride    = sizeof(Vertex),
						.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
					},
			};
			// @TODO: Fix shadow map vertex input attributes (only needs position).
			VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {
				.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
				.vertexBindingDescriptionCount   = ArrayCount(vertex_input_binding_descriptions),
				.pVertexBindingDescriptions      = vertex_input_binding_descriptions,
				.vertexAttributeDescriptionCount = ArrayCount(vertex_input_attribute_descriptions),
				.pVertexAttributeDescriptions    = vertex_input_attribute_descriptions,
			};
			VkDynamicState dynamic_states[] = {
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_SCISSOR,
				VK_DYNAMIC_STATE_DEPTH_BIAS,
			};
			VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
				.dynamicStateCount = ArrayCount(dynamic_states),
				.pDynamicStates = dynamic_states,
			};
			VkPipelineShaderStageCreateInfo shader_stage_create_infos[MAX_SHADER_MODULES] = {};
			for (s32 j = 0; j < vulkan_context.shaders[SHADOW_MAP_STATIC_SHADER].module_count; j++) {
				shader_stage_create_infos[j].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				shader_stage_create_infos[j].stage  = vulkan_context.shaders[SHADOW_MAP_STATIC_SHADER].modules[j].stage;
				shader_stage_create_infos[j].module = vulkan_context.shaders[SHADOW_MAP_STATIC_SHADER].modules[j].module;
				shader_stage_create_infos[j].pName  = "main";
			}

			VkPipelineColorBlendStateCreateInfo shadow_map_color_blend_state_create_info = color_blend_state_create_info;
			shadow_map_color_blend_state_create_info.attachmentCount = 0;

			VkPipelineDepthStencilStateCreateInfo shadow_map_depth_stencil_state_create_info = depth_stencil_state_create_info;
			shadow_map_depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

			VkPipelineRasterizationStateCreateInfo shadow_map_rasterization_state_create_info = rasterization_state_create_info;
			shadow_map_rasterization_state_create_info.depthBiasEnable = VK_TRUE;

			VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {
				.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
				.stageCount          = 1,
				.pStages             = shader_stage_create_infos,
				.pVertexInputState   = &vertex_input_state_create_info,
				.pInputAssemblyState = &input_assembly_create_info,
				.pViewportState      = &viewport_state_create_info,
				.pRasterizationState = &shadow_map_rasterization_state_create_info,
				.pMultisampleState   = &multisample_state_create_info,
				.pColorBlendState    = &shadow_map_color_blend_state_create_info,
				.pDepthStencilState  = &shadow_map_depth_stencil_state_create_info,
				.pDynamicState       = &dynamic_state_create_info,
				.layout              = vulkan_context.pipeline_layouts.shadow_map,
				.renderPass          = vulkan_context.render_passes.shadow_map,
				.subpass             = 0,
				.basePipelineIndex   = -1,
				.basePipelineHandle  = VK_NULL_HANDLE,
			};
			VK_CHECK(vkCreateGraphicsPipelines(vulkan_context.device, NULL, 1, &graphics_pipeline_create_info, NULL, &vulkan_context.pipelines.shadow_map_static));
		}

		// Flat color
		{
			VkVertexInputAttributeDescription vertex_input_attribute_descriptions[] = {
				{
					.binding  = VULKAN_VERTEX_BUFFER_BIND_ID,
					.location = 0,
					.format   = VK_FORMAT_R32G32B32_SFLOAT,
					.offset   = offsetof(Vertex1P, position),
				},
			};
			VkVertexInputBindingDescription vertex_input_binding_descriptions[] = {
					{
						.binding   = VULKAN_VERTEX_BUFFER_BIND_ID,
						.stride    = sizeof(Vertex1P),
						.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
					},
			};
			// @TODO: Fix shadow map vertex input attributes (only needs position).
			VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {
				.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
				.vertexBindingDescriptionCount   = ArrayCount(vertex_input_binding_descriptions),
				.pVertexBindingDescriptions      = vertex_input_binding_descriptions,
				.vertexAttributeDescriptionCount = ArrayCount(vertex_input_attribute_descriptions),
				.pVertexAttributeDescriptions    = vertex_input_attribute_descriptions,
			};
			VkDynamicState dynamic_states[] = {
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_SCISSOR,
			};
			VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
				.dynamicStateCount = ArrayCount(dynamic_states),
				.pDynamicStates = dynamic_states,
			};
			VkPipelineShaderStageCreateInfo shader_stage_create_infos[MAX_SHADER_MODULES] = {};
			for (s32 j = 0; j < vulkan_context.shaders[FLAT_COLOR_SHADER].module_count; j++) {
				shader_stage_create_infos[j].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				shader_stage_create_infos[j].stage  = vulkan_context.shaders[FLAT_COLOR_SHADER].modules[j].stage;
				shader_stage_create_infos[j].module = vulkan_context.shaders[FLAT_COLOR_SHADER].modules[j].module;
				shader_stage_create_infos[j].pName  = "main";
			}

			VkPipelineInputAssemblyStateCreateInfo flat_color_input_assembly_create_info = input_assembly_create_info;
			flat_color_input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

			VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {
				.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
				.stageCount          = vulkan_context.shaders[FLAT_COLOR_SHADER].module_count,
				.pStages             = shader_stage_create_infos,
				.pVertexInputState   = &vertex_input_state_create_info,
				.pInputAssemblyState = &flat_color_input_assembly_create_info,
				.pViewportState      = &viewport_state_create_info,
				.pRasterizationState = &rasterization_state_create_info,
				.pMultisampleState   = &multisample_state_create_info,
				.pColorBlendState    = &color_blend_state_create_info,
				.pDepthStencilState  = &depth_stencil_state_create_info,
				.pDynamicState       = &dynamic_state_create_info,
				.layout              = vulkan_context.pipeline_layouts.flat_color,
				.renderPass          = vulkan_context.render_pass,
				.subpass             = 0,
				.basePipelineIndex   = -1,
				.basePipelineHandle  = VK_NULL_HANDLE,
			};
			VK_CHECK(vkCreateGraphicsPipelines(vulkan_context.device, NULL, 1, &graphics_pipeline_create_info, NULL, &vulkan_context.pipelines.flat_color));
		}
	}

	// Depth image.
	{
		// @TODO: Check to make sure the physical device supports the depth image format. Have fallback formats?
		VkImageCreateInfo image_create_info = {
			.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType     = VK_IMAGE_TYPE_2D,
			.extent.width  = vulkan_context.swapchain_image_extent.width,
			.extent.height = vulkan_context.swapchain_image_extent.height,
			.extent.depth  = 1,
			.mipLevels     = 1,
			.arrayLayers   = 1,
			.format        = VK_FORMAT_D32_SFLOAT_S8_UINT,
			.tiling        = VK_IMAGE_TILING_OPTIMAL,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.usage         = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			.sharingMode   = VK_SHARING_MODE_EXCLUSIVE,
			.samples       = VK_SAMPLE_COUNT_1_BIT,
		};
		VK_CHECK(vkCreateImage(vulkan_context.device, &image_create_info, NULL, &vulkan_context.depthImage));

		VkMemoryRequirements image_memory_requirements;
		vkGetImageMemoryRequirements(vulkan_context.device, vulkan_context.depthImage, &image_memory_requirements);
		allocate_vulkan_memory(&vulkan_context.depthImageMemory, image_memory_requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		vkBindImageMemory(vulkan_context.device, vulkan_context.depthImage, vulkan_context.depthImageMemory, 0);

		VkImageViewCreateInfo image_view_create_info = {
			.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image                           = vulkan_context.depthImage,
			.viewType                        = VK_IMAGE_VIEW_TYPE_2D,
			.format                          = VK_FORMAT_D32_SFLOAT_S8_UINT,
			.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY,
			.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY,
			.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY,
			.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY,
			.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT,
			.subresourceRange.baseMipLevel   = 0,
			.subresourceRange.levelCount     = 1,
			.subresourceRange.baseArrayLayer = 0,
			.subresourceRange.layerCount     = 1,
		};
		VK_CHECK(vkCreateImageView(vulkan_context.device, &image_view_create_info, NULL, &vulkan_context.depthImageView));

		VkCommandBuffer command_buffer = Create_Single_Use_Vulkan_Command_Buffer(vulkan_context.command_pool);
		Transition_Vulkan_Image_Layout(command_buffer, vulkan_context.depthImage, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		// @TODO: Fence?
		Submit_Single_Use_Vulkan_Command_Buffer(command_buffer);
	}

	// Framebuffers.
	{
		for (s32 i = 0; i < vulkan_context.num_swapchain_images; ++i) {
			VkImageView attachments[] = {
				vulkan_context.swapchain_image_views[i],
				vulkan_context.depthImageView,
			};
			VkFramebufferCreateInfo framebuffer_create_info = {
				.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				.renderPass      = vulkan_context.render_pass,
				.attachmentCount = ArrayCount(attachments),
				.pAttachments    = attachments,
				.width           = vulkan_context.swapchain_image_extent.width,
				.height          = vulkan_context.swapchain_image_extent.height,
				.layers          = 1,
			};
			VK_CHECK(vkCreateFramebuffer(vulkan_context.device, &framebuffer_create_info, NULL, &vulkan_context.framebuffers[i]));
		}
	}

	{
		// For shadow mapping we only need a depth attachment
		VkImageCreateInfo image = {
			.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType     = VK_IMAGE_TYPE_2D,
			.extent.width  = SHADOW_MAP_WIDTH,
			.extent.height = SHADOW_MAP_HEIGHT,
			.extent.depth  = 1,
			.mipLevels     = 1,
			.arrayLayers   = 1,
			.samples       = VK_SAMPLE_COUNT_1_BIT,
			.tiling        = VK_IMAGE_TILING_OPTIMAL,
			.format        = SHADOW_MAP_DEPTH_FORMAT,
			.usage         = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		};
		VK_CHECK(vkCreateImage(vulkan_context.device, &image, NULL, &vulkan_context.framebuffer_attachments.shadow_map_depth.image));

		VkMemoryRequirements memReqs;
		vkGetImageMemoryRequirements(vulkan_context.device, vulkan_context.framebuffer_attachments.shadow_map_depth.image, &memReqs);
		allocate_vulkan_memory(&vulkan_context.framebuffer_attachments.shadow_map_depth.memory, memReqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VK_CHECK(vkBindImageMemory(vulkan_context.device, vulkan_context.framebuffer_attachments.shadow_map_depth.image, vulkan_context.framebuffer_attachments.shadow_map_depth.memory, 0));

		VkImageViewCreateInfo depthStencilView = {
			.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.viewType                        = VK_IMAGE_VIEW_TYPE_2D,
			.format                          = SHADOW_MAP_DEPTH_FORMAT,
			.subresourceRange                = {},
			.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT,
			.subresourceRange.baseMipLevel   = 0,
			.subresourceRange.levelCount     = 1,
			.subresourceRange.baseArrayLayer = 0,
			.subresourceRange.layerCount     = 1,
			.image                           = vulkan_context.framebuffer_attachments.shadow_map_depth.image,
		};
		VK_CHECK(vkCreateImageView(vulkan_context.device, &depthStencilView, NULL, &vulkan_context.framebuffer_attachments.shadow_map_depth.image_view));

		// Create sampler to sample from to depth attachment 
		// Used to sample in the fragment shader for shadowed rendering
		VkSamplerCreateInfo sampler_create_info = {};
		sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_create_info.magFilter = SHADOW_MAP_FILTER;
		sampler_create_info.minFilter = SHADOW_MAP_FILTER;
		sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler_create_info.addressModeV = sampler_create_info.addressModeU;
		sampler_create_info.addressModeW = sampler_create_info.addressModeU;
		sampler_create_info.mipLodBias = 0.0f;
		sampler_create_info.maxAnisotropy = 1.0f;
		sampler_create_info.minLod = 0.0f;
		sampler_create_info.maxLod = 1.0f;
		sampler_create_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		VK_CHECK(vkCreateSampler(vulkan_context.device, &sampler_create_info, NULL, &vulkan_context.samplers.shadow_map_depth));

		// Create frame buffer
		VkFramebufferCreateInfo framebuffer_create_info = {};
		framebuffer_create_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_create_info.renderPass = vulkan_context.render_passes.shadow_map;
		framebuffer_create_info.attachmentCount = 1;
		framebuffer_create_info.pAttachments = &vulkan_context.framebuffer_attachments.shadow_map_depth.image_view;
		framebuffer_create_info.width = SHADOW_MAP_WIDTH;
		framebuffer_create_info.height = SHADOW_MAP_WIDTH;
		framebuffer_create_info.layers = 1;

		VK_CHECK(vkCreateFramebuffer(vulkan_context.device, &framebuffer_create_info, NULL, &vulkan_context.xframebuffers.shadow_map));
	}
}

void create_vulkan_command_buffers() {
	VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
	command_buffer_allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_info.commandPool        = vulkan_context.command_pool;
	command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_allocate_info.commandBufferCount = vulkan_context.num_swapchain_images;
	VK_CHECK(vkAllocateCommandBuffers(vulkan_context.device, &command_buffer_allocate_info, vulkan_context.command_buffers));
}

void create_vulkan_buffer(VkBuffer *buffer, VkDeviceMemory *memory, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags desired_memory_properties) {
	VkBufferCreateInfo buffer_create_info = {};
	buffer_create_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_create_info.size        = size;
	buffer_create_info.usage       = usage;
	buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK(vkCreateBuffer(vulkan_context.device, &buffer_create_info, NULL, buffer));

	VkMemoryRequirements memory_requirements;
	vkGetBufferMemoryRequirements(vulkan_context.device, *buffer, &memory_requirements);

	allocate_vulkan_memory(memory,  memory_requirements, desired_memory_properties);
	VK_CHECK(vkBindBufferMemory(vulkan_context.device, *buffer, *memory, 0));
}

void stage_vulkan_data(void *data, u32 size) {
	Assert(vulkan_context.staging_memory_bytes_used + size < VULKAN_STAGING_MEMORY_SEGMENT_SIZE);

	// @TODO: Keep shared memory mapped permanantly?
	void *shared_memory;
	VK_CHECK(vkMapMemory(vulkan_context.device, vulkan_context.shared_memory, 0, VULKAN_STAGING_MEMORY_SEGMENT_SIZE, 0, &shared_memory));
	memcpy((char *)shared_memory + vulkan_context.staging_memory_bytes_used, data, size);
	vkUnmapMemory(vulkan_context.device, vulkan_context.shared_memory);
	vulkan_context.staging_memory_bytes_used += size;
}

void transfer_staged_vulkan_data(u32 offset) {
	Assert(vulkan_context.staging_memory_bytes_used + offset < VULKAN_GPU_ALLOCATION_SIZE);

	// @TODO: Try to create a reusable command buffer for copying?
    VkCommandBufferAllocateInfo command_buffer_allocate_info = {
		.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandPool        = vulkan_context.command_pool,
		.commandBufferCount = 1,
    };
    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(vulkan_context.device, &command_buffer_allocate_info, &command_buffer);

	VkCommandBufferBeginInfo command_buffer_begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};
	vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);
	VkBufferCopy copy_region = {
		.srcOffset = VULKAN_STAGING_MEMORY_SEGMENT_OFFSET,
		.dstOffset = offset,
		.size      = vulkan_context.staging_memory_bytes_used,
	};
	vkCmdCopyBuffer(command_buffer, vulkan_context.staging_buffer, vulkan_context.gpu_buffer, 1, &copy_region);
	vkEndCommandBuffer(command_buffer);

	VkSubmitInfo submit_info = {
		.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers    = &command_buffer,
	};
	VK_CHECK(vkQueueSubmit(vulkan_context.graphics_queue, 1, &submit_info, VK_NULL_HANDLE));
	VK_CHECK(vkQueueWaitIdle(vulkan_context.graphics_queue)); // @TODO: Use a fence?
	vkFreeCommandBuffers(vulkan_context.device, vulkan_context.command_pool, 1, &command_buffer);

	vulkan_context.staging_memory_bytes_used = 0;
}

void copy_vulkan_buffer(VkBuffer source, VkBuffer destination, VkDeviceSize size, u32 source_offset, u32 destination_offset) {
	// @TODO: Try to create a reusable command buffer for copying?
    VkCommandBufferAllocateInfo allocate_info = {};
    allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandPool        = vulkan_context.command_pool;
    allocate_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(vulkan_context.device, &allocate_info, &command_buffer);

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(command_buffer, &begin_info);
	VkBufferCopy copy_region = {};
	copy_region.srcOffset = source_offset;
	copy_region.dstOffset = destination_offset;
	copy_region.size      = size;
	vkCmdCopyBuffer(command_buffer, source, destination, 1, &copy_region);
	vkEndCommandBuffer(command_buffer);

	VkSubmitInfo submit_info = {};
	submit_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers    = &command_buffer;

	vkQueueSubmit(vulkan_context.graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(vulkan_context.graphics_queue); // @TODO: Use a fence?
	vkFreeCommandBuffers(vulkan_context.device, vulkan_context.command_pool, 1, &command_buffer);
}

void Copy_Vulkan_Buffer(VkCommandBuffer command_buffer, VkBuffer source, VkBuffer destination, VkDeviceSize size) {
	VkBufferCopy copy_region = {
		.size = size,
	};
	vkCmdCopyBuffer(command_buffer, source, destination, 1, &copy_region);
}

void Copy_Vulkan_Buffer_To_Image(VkCommandBuffer command_buffer, VkBuffer buffer, VkImage image, u32 width, u32 height) {
	VkBufferImageCopy region = {
		.bufferOffset = 0,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,
		.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.imageSubresource.mipLevel = 0,
		.imageSubresource.baseArrayLayer = 0,
		.imageSubresource.layerCount = 1,
		.imageOffset = {0, 0, 0},
		.imageExtent = {
			width,
			height,
			1,
		},
	};
	vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void vulkan_push_debug_vertices(void *vertices, u32 vertex_count, u32 sizeof_vertex, u32 *indices, u32 index_count, u32 *vertex_offset, u32 *first_index) {
	u32 indices_size = index_count * sizeof(u32);
	u32 vertices_size = vertex_count * sizeof_vertex;

	Assert(vulkan_context.debug_vertex_memory_bytes_used + vertices_size < VULKAN_FRAME_VERTEX_MEMORY_SEGMENT_SIZE);
	Assert(vulkan_context.debug_index_memory_bytes_used + indices_size < VULKAN_FRAME_INDEX_MEMORY_SEGMENT_SIZE);

	void *shared_memory;
	VK_CHECK(vkMapMemory(vulkan_context.device, vulkan_context.shared_memory, VULKAN_FRAME_VERTEX_MEMORY_SEGMENT_OFFSET + (vulkan_context.currentFrame * VULKAN_FRAME_VERTEX_MEMORY_SEGMENT_SIZE), VULKAN_FRAME_VERTEX_MEMORY_SEGMENT_SIZE, 0, &shared_memory));
	memcpy((char *)shared_memory + vulkan_context.debug_vertex_memory_bytes_used, vertices, vertices_size);
	vkUnmapMemory(vulkan_context.device, vulkan_context.shared_memory);
	*vertex_offset = vulkan_context.debug_vertex_count;
	vulkan_context.debug_vertex_count += vertex_count;
	vulkan_context.debug_vertex_memory_bytes_used += vertices_size;

	VK_CHECK(vkMapMemory(vulkan_context.device, vulkan_context.shared_memory, VULKAN_FRAME_INDEX_MEMORY_SEGMENT_OFFSET + (vulkan_context.currentFrame * VULKAN_FRAME_INDEX_MEMORY_SEGMENT_SIZE), VULKAN_FRAME_INDEX_MEMORY_SEGMENT_SIZE, 0, &shared_memory));
	memcpy((char *)shared_memory + vulkan_context.debug_index_memory_bytes_used, indices, indices_size);
	vkUnmapMemory(vulkan_context.device, vulkan_context.shared_memory);
	*first_index = vulkan_context.debug_index_count;
	vulkan_context.debug_index_count += index_count;
	vulkan_context.debug_index_memory_bytes_used += indices_size;
}

TextureID GPU_Upload_Texture(GPU_Context *context, u8 *pixels, s32 texture_width, s32 texture_height, GPU_Upload_Flags gpu_upload_flags) {
	TextureID id = vulkan_context.textures.id_generator++;

	VkImageCreateInfo image_create_info = {
		.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType     = VK_IMAGE_TYPE_2D,
		.extent.width  = texture_width,
		.extent.height = texture_height,
		.extent.depth  = 1,
		.mipLevels     = 1,
		.arrayLayers   = 1,
		.format        = VK_FORMAT_R8G8B8A8_UNORM,
		.tiling        = VK_IMAGE_TILING_OPTIMAL,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		.sharingMode   = VK_SHARING_MODE_EXCLUSIVE,
		.samples       = VK_SAMPLE_COUNT_1_BIT,
	};
	Assert(id < VULKAN_MAX_TEXTURES);
	VK_CHECK(vkCreateImage(vulkan_context.device, &image_create_info, NULL, &vulkan_context.textures.images[id]));
	VkImage image = vulkan_context.textures.images[id];

	VkMemoryRequirements memory_requirements;
	vkGetImageMemoryRequirements(vulkan_context.device, image, &memory_requirements);
	u32 image_size = memory_requirements.size;
	u32 sneed = align_to(vulkan_context.image_memory_bytes_used + image_size, memory_requirements.alignment);
	debug_print("ASD %u\n", image_size);
	//debug_print("%u %u\n", alignment_offset, image_size);
	Assert(vulkan_context.image_memory_bytes_used + image_size < VULKAN_IMAGE_ALLOCATION_SIZE);
	//vulkan_context.image_memory_bytes_used += alignemnt_offset + image_size;

	//VkDeviceSize image_size = texture_width * texture_height * 4;
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	create_vulkan_buffer(&stagingBuffer, &stagingBufferMemory, image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	void* data;
	VK_CHECK(vkMapMemory(vulkan_context.device, stagingBufferMemory, 0, image_size, 0, &data));
	memcpy(data, pixels, image_size);
	vkUnmapMemory(vulkan_context.device, stagingBufferMemory);

	vkBindImageMemory(vulkan_context.device, image, vulkan_context.image_memory, vulkan_context.image_memory_bytes_used);
	VkCommandBuffer command_buffer = Create_Single_Use_Vulkan_Command_Buffer(context->thread_local[thread_index].command_pools[vulkan_context.currentFrame]);
	Transition_Vulkan_Image_Layout(command_buffer, image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	Copy_Vulkan_Buffer_To_Image(command_buffer, stagingBuffer, image, texture_width, texture_height);
	Transition_Vulkan_Image_Layout(command_buffer, image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	VkFence fence;
	if (gpu_upload_flags & GPU_UPLOAD_AS_SOON_AS_POSSIBLE) {
		Assert(0);
		// @TODO: BROKEN. Can still submit on multiple threads at the same time.
		fence = Submit_Single_Use_Vulkan_Command_Buffer(command_buffer);
	} else {
		vkEndCommandBuffer(command_buffer);
		// @TODO Atomic_Write_To_Ring_Buffer(vulkan_context.asset_upload_command_buffers, command_buffer);
	}

	//VkFence fence = Finish_Single_Use_Vulkan_Command_Buffer(command_buffer);

	// @TODO: vkDestroyBuffer(vulkan_context.device, stagingBuffer, NULL);
	// @TODO: vkFreeMemory(vulkan_context.device, stagingBufferMemory, NULL);

	vulkan_context.image_memory_bytes_used = sneed;

	VkImageViewCreateInfo image_view_create_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = VK_FORMAT_R8G8B8A8_UNORM,
		.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.subresourceRange.baseMipLevel = 0,
		.subresourceRange.levelCount = 1,
		.subresourceRange.baseArrayLayer = 0,
		.subresourceRange.layerCount = 1,
	};
	VK_CHECK(vkCreateImageView(vulkan_context.device, &image_view_create_info, NULL, &vulkan_context.textures.image_views[id]));

	// Update texture descriptors.
	{
		// @TODO: Only upload these if they need to be updated.
		// @TODO: Use a dummy texture for all descriptors not in use? All image_infos at index greater than vulkan_context.textures.count.
		// @TODO: We should set all texture descriptors to a dummy texture on startup and then only update the ones that actually need to be updated!
		//VkDescriptorImageInfo *image_infos = allocate_array(&game_state->frame_arena, VkDescriptorImageInfo, VULKAN_MAX_TEXTURES);
		//u32 total_mesh_count = 0;
		//for (u32 i = 0; i < game_state->entities.mesh_count; i++) {
			//for (u32 j = 0; j < game_state->entities.submesh_counts[i]; j++) {
		//image_infos[total_mesh_count].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		//image_infos[total_mesh_count].imageView   = vulkan_context.textures.image_views[id];
		//image_infos[total_mesh_count].sampler     = NULL;
		//total_mesh_count++;
			//}
		//}
		//for (u32 i = total_mesh_count; i < VULKAN_MAX_TEXTURES; i++) {
			//image_infos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			//image_infos[i].imageView   = vulkan_context.textures.image_views[0];
			//image_infos[i].sampler     = NULL;
		//}
		VkWriteDescriptorSet textures_descriptor_write = {
			.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstBinding      = 4,
			.dstArrayElement = vulkan_context.textures.count, // @TODO: Will need to change once we start unloading textures.
			.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			.descriptorCount = 1,
			.pBufferInfo     = 0,
			.dstSet          = vulkan_context.descriptor_sets.scene.texture,
			.pImageInfo      = &(VkDescriptorImageInfo){
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				.imageView   = vulkan_context.textures.image_views[id],
				.sampler     = NULL,
			},
		};
		vkUpdateDescriptorSets(vulkan_context.device, 1, &textures_descriptor_write, 0, NULL);
	}

	vulkan_context.textures.count++;

	return id;
}
#endif

void Render_API_Record_Begin_Render_Pass_Command(Render_API_Context *context, GPU_Command_Buffer command_buffer, GPU_Render_Pass render_pass, GPU_Framebuffer framebuffer) {
	VkClearValue clear_color = {{{0.04f, 0.19f, 0.34f, 1.0f}}};
	VkClearValue clear_depth_stencil = {{{1.0f, 0.0f}}};
	VkClearValue clear_values[] = {
		clear_color,
		clear_depth_stencil,
	};
	VkRenderPassBeginInfo render_pass_begin_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = render_pass,
		.framebuffer = framebuffer,
		.renderArea = {
			.offset = {0, 0},
			.extent = {window_width, window_height},
		},
		.clearValueCount = ArrayCount(clear_values),
		.pClearValues = clear_values,
	};
	vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void Render_API_Record_End_Render_Pass_Command(Render_API_Context *context, GPU_Command_Buffer command_buffer) {
	vkCmdEndRenderPass(command_buffer);
}

void Render_API_Record_Set_Viewport_Command(Render_API_Context *context, GPU_Command_Buffer command_buffer, s32 width, s32 height) {
	VkViewport viewport = {
		.x = 0.0f,
		.y = 0.0f,
		.width = (f32)width,
		.height = (f32)height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};
	vkCmdSetViewport(command_buffer, 0, 1, &viewport);
}

void Render_API_Record_Set_Scissor_Command(Render_API_Context *context, GPU_Command_Buffer command_buffer, u32 width, u32 height) {
	VkRect2D scissor = {
		.offset = {0, 0},
		.extent = {width, height},
	};
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);
}

void Render_API_Record_Bind_Pipeline_Command(Render_API_Context *context, GPU_Command_Buffer command_buffer, GPU_Pipeline pipeline) {
	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
}

#define VULKAN_VERTEX_BUFFER_BIND_ID 0

void Render_API_Record_Bind_Vertex_Buffer_Command(Render_API_Context *context, GPU_Command_Buffer command_buffer, GPU_Buffer vertex_buffer) {
	VkDeviceSize DeviceSize = 0;
	vkCmdBindVertexBuffers(command_buffer, VULKAN_VERTEX_BUFFER_BIND_ID, 1, &vertex_buffer, &DeviceSize);
}

void Render_API_Record_Bind_Index_Buffer_Command(Render_API_Context *context, GPU_Command_Buffer command_buffer, GPU_Buffer index_buffer) {
	vkCmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);
}

void Render_API_Draw_Indexed_Vertices(Render_API_Context *context, GPU_Command_Buffer command_buffer, u32 index_count, u32 first_index) {
	vkCmdDrawIndexed(command_buffer, index_count, 1, first_index, 0, 0);
}

//TextureID load_texture(String path);

typedef struct {
	Shaders *shaders;
	const char **paths;
	u32 shader_module_count;
} Create_Shader_Request;

void Render_API_Initialize(Render_API_Context *context) {
	PlatformDynamicLibraryHandle vulkan_library = PlatformOpenDynamicLibrary("libvulkan.so");
#define VK_EXPORTED_FUNCTION(name) \
	name = (PFN_##name)PlatformGetDynamicLibraryFunction(vulkan_library, #name); \
	if (!name) Abort("Failed to load Vulkan function %s: Vulkan version 1.1 required", #name);
#define VK_GLOBAL_FUNCTION(name) \
	name = (PFN_##name)vkGetInstanceProcAddr(NULL, (const char *)#name); \
	if (!name) Abort("Failed to load Vulkan function %s: Vulkan version 1.1 required", #name);
#define VK_INSTANCE_FUNCTION(name)
#define VK_DEVICE_FUNCTION(name)
#include "VulkanFunctions.h"
#undef VK_EXPORTED_FUNCTION
#undef VK_GLOBAL_FUNCTION
#undef VK_INSTANCE_FUNCTION
#undef VK_DEVICE_FUNCTION

	const char *required_device_extensions[] = {
		"VK_KHR_swapchain",
		"VK_EXT_descriptor_indexing",
	};
	const char *required_instance_layers[] = {
#if defined(DEBUG)
		"VK_LAYER_KHRONOS_validation",
#endif
	};
	const char *required_instance_extensions[] = {
		"VK_KHR_surface",
		PlatformGetRequiredVulkanSurfaceInstanceExtension(),
#if defined(DEBUG)
		"VK_EXT_debug_utils",
#endif
	};

	u32 version;
	vkEnumerateInstanceVersion(&version);
	if (VK_VERSION_MAJOR(version) < 1 || (VK_VERSION_MAJOR(version) == 1 && VK_VERSION_MINOR(version) < 1)) {
		Abort("Vulkan version 1.1 or greater required: version %d.%d.%d is installed");
	}
	LogPrint(INFO_LOG, "Using Vulkan version %d.%d.%d\n", VK_VERSION_MAJOR(version), VK_VERSION_MINOR(version), VK_VERSION_PATCH(version));

	VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {
#if defined(DEBUG)
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = VulkanDebugMessageCallback,
#endif
	};

	// Create instance.
	{
		u32 available_instance_layer_count;
		vkEnumerateInstanceLayerProperties(&available_instance_layer_count, NULL);
		VkLayerProperties available_instance_layers[available_instance_layer_count];
		vkEnumerateInstanceLayerProperties(&available_instance_layer_count, available_instance_layers);
		LogPrint(INFO_LOG, "Available Vulkan layers:\n");
		for (s32 i = 0; i < available_instance_layer_count; i++) {
			LogPrint(INFO_LOG, "\t%s\n", available_instance_layers[i].layerName);
		}
		u32 available_instance_extension_count = 0;
		VK_CHECK(vkEnumerateInstanceExtensionProperties(NULL, &available_instance_extension_count, NULL));
		VkExtensionProperties available_instance_extensions[available_instance_extension_count];
		VK_CHECK(vkEnumerateInstanceExtensionProperties(NULL, &available_instance_extension_count, available_instance_extensions));
		LogPrint(INFO_LOG, "Available Vulkan instance extensions:\n");
		for (s32 i = 0; i < available_instance_extension_count; i++) {
			LogPrint(INFO_LOG, "\t%s\n", available_instance_extensions[i].extensionName);
		}
		auto ApplicationInfo = VkApplicationInfo{
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName = "Jaguar",
			.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
			.pEngineName = "Jaguar",
			.engineVersion = VK_MAKE_VERSION(1, 0, 0),
			.apiVersion = VK_MAKE_VERSION(1, 1, 0),
		};
		// @TODO: Check that all required extensions/layers are available.
		VkInstanceCreateInfo instance_create_info = {
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
#if defined(DEBUG)
			.pNext = &debug_create_info,
#endif
			.pApplicationInfo = &ApplicationInfo,
			.enabledLayerCount = ArrayCount(required_instance_layers),
			.ppEnabledLayerNames = required_instance_layers,
			.enabledExtensionCount = ArrayCount(required_instance_extensions),
			.ppEnabledExtensionNames = required_instance_extensions,
		};
		VK_CHECK(vkCreateInstance(&instance_create_info, NULL, &context->instance));
	}

#define VK_EXPORTED_FUNCTION(name)
#define VK_GLOBAL_FUNCTION(name)
#define VK_INSTANCE_FUNCTION(name) \
	name = (PFN_##name)vkGetInstanceProcAddr(context->instance, (const char *)#name); \
	if (!name) Abort("Failed to load Vulkan function %s: Vulkan version 1.1 required", #name);
#define VK_DEVICE_FUNCTION(name)
#include "VulkanFunctions.h"
#undef VK_EXPORTED_FUNCTION
#undef VK_GLOBAL_FUNCTION
#undef VK_INSTANCE_FUNCTION
#undef VK_DEVICE_FUNCTION

	if (debug) {
		VK_CHECK(vkCreateDebugUtilsMessengerEXT(context->instance, &debug_create_info, NULL, &context->debug_messenger));
	}

	PlatformWindow window;
	PlatformCreateVulkanSurface(window, context->instance, &context->window_surface);

	// Select physical device.
	// @TODO: Rank physical device and select the best one?
	{
		bool found_suitable_physical_device = false;
		u32 available_physical_device_count = 0;
		VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &available_physical_device_count, NULL));
		if (available_physical_device_count == 0) {
			Abort("Could not find any physical devices.");
		}
		VkPhysicalDevice available_physical_devices[available_physical_device_count];
		VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &available_physical_device_count , available_physical_devices));
		for (s32 i = 0; i < available_physical_device_count; i++) {
			VkPhysicalDeviceProperties physical_device_properties;
			vkGetPhysicalDeviceProperties(available_physical_devices[i], &physical_device_properties);
			VkPhysicalDeviceFeatures physical_device_features;
			vkGetPhysicalDeviceFeatures(available_physical_devices[i], &physical_device_features);
			if (!physical_device_features.samplerAnisotropy || !physical_device_features.shaderSampledImageArrayDynamicIndexing) {
				continue;
			}
			u32 available_device_extension_count = 0;
			VK_CHECK(vkEnumerateDeviceExtensionProperties(available_physical_devices[i], NULL, &available_device_extension_count, NULL));
			VkExtensionProperties available_device_extensions[available_device_extension_count];
			VK_CHECK(vkEnumerateDeviceExtensionProperties(available_physical_devices[i], NULL, &available_device_extension_count, available_device_extensions));
			bool missing_required_device_extension = false;
			for (s32 j = 0; j < ArrayCount(required_device_extensions); j++) {
				bool found = false;
				for (s32 k = 0; k < available_device_extension_count; k++) {
					if (strcmp(available_device_extensions[k].extensionName, required_device_extensions[j]) == 0) { // @TODO
						found = true;
						break;
					}
				}
				if (!found) {
					missing_required_device_extension = true;
					break;
				}
			}
			if (missing_required_device_extension) {
				continue;
			}
			// Make sure the swap chain is compatible with our window surface.
			// If we have at least one supported surface format and present mode, we will consider the device.
			// @TODO: Should we die if error, or just skip this physical device?
			u32 available_surface_format_count = 0;
			VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(available_physical_devices[i], context->window_surface, &available_surface_format_count, NULL));
			u32 available_present_mode_count = 0;
			VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(available_physical_devices[i], context->window_surface, &available_present_mode_count, NULL));
			if (available_surface_format_count == 0 || available_present_mode_count == 0) {
				continue;
			}
			// Select the best swap chain settings.
			VkSurfaceFormatKHR available_surface_formats[available_surface_format_count];
			VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(available_physical_devices[i], context->window_surface, &available_surface_format_count, available_surface_formats));
			VkSurfaceFormatKHR surface_format = available_surface_formats[0];
			if (available_surface_format_count == 1 && available_surface_formats[0].format == VK_FORMAT_UNDEFINED) {
				// No preferred format, so we get to pick our own.
				surface_format = (VkSurfaceFormatKHR){VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
			} else {
				for (s32 j = 0; j < available_surface_format_count; j++) {
					if (available_surface_formats[j].format == VK_FORMAT_B8G8R8A8_UNORM && available_surface_formats[j].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
						surface_format = available_surface_formats[j];
						break;
					}
				}
			}
			// VK_PRESENT_MODE_IMMEDIATE_KHR is for applications that don't care about tearing, or have some way of synchronizing their rendering with the display.
			// VK_PRESENT_MODE_MAILBOX_KHR may be useful for applications that generally render a new presentable image every refresh cycle, but are occasionally early.
			// In this case, the application wants the new image to be displayed instead of the previously-queued-for-presentation image that has not yet been displayed.
			// VK_PRESENT_MODE_FIFO_RELAXED_KHR is for applications that generally render a new presentable image every refresh cycle, but are occasionally late.
			// In this case (perhaps because of stuttering/latency concerns), the application wants the late image to be immediately displayed, even though that may mean some tearing.
			VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
			VkPresentModeKHR available_present_modes[available_present_mode_count];
			VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(available_physical_devices[i], context->window_surface, &available_present_mode_count, available_present_modes));
			for (s32 j = 0; j < available_present_mode_count; j++) {
				if (available_present_modes[j] == VK_PRESENT_MODE_MAILBOX_KHR) {
					present_mode = available_present_modes[j];
					break;
				}
			}
			u32 queue_family_count = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(available_physical_devices[i], &queue_family_count, NULL);
			VkQueueFamilyProperties queue_families[queue_family_count];
			vkGetPhysicalDeviceQueueFamilyProperties(available_physical_devices[i], &queue_family_count, queue_families);
			// @TODO: Search for an transfer exclusive queue VK_QUEUE_TRANSFER_BIT.
			s32 graphics_queue_family = -1;
			s32 present_queue_family = -1;
			for (s32 j = 0; j < queue_family_count; j++) {
				if (queue_families[j].queueCount == 0) {
					continue;
				}
				if (queue_families[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					graphics_queue_family = j;
				}
				u32 present_support = 0;
				VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(available_physical_devices[i], j, context->window_surface, &present_support));
				if (present_support) {
					present_queue_family = j;
				}
				if (graphics_queue_family != -1 && present_queue_family != -1) {
					context->physical_device = available_physical_devices[i];
					vulkanContext.physicalDevice = available_physical_devices[i];
					context->graphics_queue_family = graphics_queue_family;
					context->present_queue_family = present_queue_family;
					vulkanContext.graphicsQueueFamily = graphics_queue_family;
					vulkanContext.presentQueueFamily = present_queue_family;
					context->window_surface_format = surface_format;
					context->present_mode = present_mode;
					context->minimum_uniform_buffer_offset_alignment = physical_device_properties.limits.minUniformBufferOffsetAlignment;
					context->maximum_uniform_buffer_size = physical_device_properties.limits.maxUniformBufferRange;
					found_suitable_physical_device = true;
					LogPrint(INFO_LOG, "Available Vulkan device extensions:\n");
					for (s32 k = 0; k < available_device_extension_count; k++) {
						LogPrint(INFO_LOG, "\t%s\n", available_device_extensions[k].extensionName);
					}
					break;
				}
			}
			if (found_suitable_physical_device) {
				break;
			}
		}
		if (!found_suitable_physical_device) {
			Abort("Could not find suitable physical device.\n");
		}
	}

	// Create logical device.
	{
		u32 unique_queue_family_count = (context->graphics_queue_family == context->present_queue_family) ? 1 : 2;
		f32 queue_priority = 1.0f;
		VkDeviceQueueCreateInfo device_queue_create_info[unique_queue_family_count];
		device_queue_create_info[0] = (VkDeviceQueueCreateInfo){
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = context->graphics_queue_family,
			.queueCount = 1,
			.pQueuePriorities = &queue_priority,
		};
		if (unique_queue_family_count > 1) {
			device_queue_create_info[1] = (VkDeviceQueueCreateInfo){
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = context->present_queue_family,
				.queueCount = 1,
				.pQueuePriorities = &queue_priority,
			};
		}
		auto PhysicalDeviceFeatures = VkPhysicalDeviceFeatures{
			.samplerAnisotropy = VK_TRUE,
		};
		auto DescriptorIndexingFeatures = VkPhysicalDeviceDescriptorIndexingFeaturesEXT{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT,
			.runtimeDescriptorArray = VK_TRUE,
		};
		VkDeviceCreateInfo device_create_info = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = &DescriptorIndexingFeatures,
			.queueCreateInfoCount = unique_queue_family_count,
			.pQueueCreateInfos = device_queue_create_info,
			.enabledLayerCount = ArrayCount(required_instance_layers),
			.ppEnabledLayerNames = required_instance_layers,
			.enabledExtensionCount = ArrayCount(required_device_extensions),
			.ppEnabledExtensionNames = required_device_extensions,
			.pEnabledFeatures = &PhysicalDeviceFeatures,
		};
		VK_CHECK(vkCreateDevice(context->physical_device, &device_create_info, NULL, &context->device));
		VK_CHECK(vkCreateDevice(context->physical_device, &device_create_info, NULL, &vulkanContext.device));
	}

#define VK_EXPORTED_FUNCTION(name)
#define VK_GLOBAL_FUNCTION(name)
#define VK_INSTANCE_FUNCTION(name)
#define VK_DEVICE_FUNCTION(name) \
	name = (PFN_##name)vkGetDeviceProcAddr(context->device, (const char *)#name); \
	if (!name) Abort("Failed to load Vulkan function %s: Vulkan version 1.1 required", #name);
#include "VulkanFunctions.h"
#undef VK_EXPORTED_FUNCTION
#undef VK_GLOBAL_FUNCTION
#undef VK_INSTANCE_FUNCTION
#undef VK_DEVICE_FUNCTION

	vkGetDeviceQueue(context->device, context->graphics_queue_family, 0, &context->graphics_queue); // No return.
	vkGetDeviceQueue(context->device, context->present_queue_family, 0, &context->present_queue); // No return.

	vkGetDeviceQueue(vulkanContext.device, vulkanContext.graphicsQueueFamily, 0, &vulkanContext.graphicsQueue); // No return.
	vkGetDeviceQueue(vulkanContext.device, vulkanContext.presentQueueFamily, 0, &vulkanContext.presentQueue); // No return.

	// Create presentation semaphores.
	{
		VkSemaphoreCreateInfo semaphore_create_info = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		};
		VkFenceCreateInfo fence_create_info = {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT,
		};
		for (s32 i = 0; i < GPU_MAX_FRAMES_IN_FLIGHT; i++) {
			VK_CHECK(vkCreateSemaphore(context->device, &semaphore_create_info, NULL, &context->image_available_semaphores[i]));
			VK_CHECK(vkCreateSemaphore(context->device, &semaphore_create_info, NULL, &context->render_finished_semaphores[i]));
		}
	}

#if 0
	// Descriptor set layout.
	{
		VkDescriptorSetLayoutBinding uniform_buffer_layout_binding = {
			.binding            = 0,
			.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount    = 1,
			.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = NULL,
		};
		VkDescriptorSetLayoutBinding scene_material_layout_binding = {
			.binding            = 5,
			.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount    = 1,
			.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = NULL,
		};
		VkDescriptorSetLayoutBinding dynamic_uniform_buffer_layout_binding = {
			.binding            = 1,
			.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			.descriptorCount    = 1,
			.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = NULL,
		};
		VkDescriptorSetLayoutBinding sampler_layout_binding = {
			.binding            = 2,
			.descriptorType     = VK_DESCRIPTOR_TYPE_SAMPLER,
			.descriptorCount    = 1,
			.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = NULL,
		};
		VkDescriptorSetLayoutBinding smsampler_layout_binding = {
			.binding            = 3,
			.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount    = 1,
			.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = NULL,
		};
		VkDescriptorSetLayoutBinding tex_sampler_layout_binding = {
			.binding            = 4,
			.descriptorType     = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			.descriptorCount    = VULKAN_MAX_TEXTURES,
			.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = NULL,
		};
		VkDescriptorSetLayoutBinding sm_layout_binding = {
			.binding            = 0,
			.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount    = 1,
			.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = NULL,
		};
		{
			VkDescriptorSetLayoutBinding bindings[] = {
				uniform_buffer_layout_binding,
			};
			VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
				.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				.bindingCount = ArrayCount(bindings),
				.pBindings    = bindings,
				.flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
			};
			VK_CHECK(vkCreateDescriptorSetLayout(vulkan_context.device, &descriptor_set_layout_create_info, NULL, &vulkan_context.descriptor_set_layouts.scene[SCENE_UNIFORM_DESCRIPTOR_SET]));
		}
		{
			VkDescriptorSetLayoutBinding bindings[] = {
				scene_material_layout_binding,
			};
			VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
				.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				.bindingCount = ArrayCount(bindings),
				.pBindings    = bindings,
			};
			VK_CHECK(vkCreateDescriptorSetLayout(vulkan_context.device, &descriptor_set_layout_create_info, NULL, &vulkan_context.descriptor_set_layouts.scene[SCENE_MATERIAL_DESCRIPTOR_SET]));
		}
		{
			VkDescriptorSetLayoutBinding bindings[] = {
				dynamic_uniform_buffer_layout_binding,
			};
			VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
				.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				.bindingCount = ArrayCount(bindings),
				.pBindings    = bindings,
			};
			VK_CHECK(vkCreateDescriptorSetLayout(vulkan_context.device, &descriptor_set_layout_create_info, NULL, &vulkan_context.descriptor_set_layouts.scene[SCENE_DYNAMIC_UNIFORM_DESCRIPTOR_SET]));
		}
		{
			VkDescriptorSetLayoutBinding bindings[] = {
				sampler_layout_binding,
				smsampler_layout_binding,
			};
			VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
				.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				.bindingCount = ArrayCount(bindings),
				.pBindings    = bindings,
			};
			VK_CHECK(vkCreateDescriptorSetLayout(vulkan_context.device, &descriptor_set_layout_create_info, NULL, &vulkan_context.descriptor_set_layouts.scene[SCENE_SAMPLER_DESCRIPTOR_SET]));
		}
		{
			VkDescriptorSetLayoutBinding bindings[] = {
				tex_sampler_layout_binding,
			};
			VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
				.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				.bindingCount = ArrayCount(bindings),
				.pBindings    = bindings,
			};
			VK_CHECK(vkCreateDescriptorSetLayout(vulkan_context.device, &descriptor_set_layout_create_info, NULL, &vulkan_context.descriptor_set_layouts.scene[SCENE_TEXTURE_DESCRIPTOR_SET]));
		}
		{
			VkDescriptorSetLayoutBinding bindings[] = {
				sm_layout_binding,
			};
			VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
				.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				.bindingCount = ArrayCount(bindings),
				.pBindings    = bindings,
			};
			VK_CHECK(vkCreateDescriptorSetLayout(vulkan_context.device, &descriptor_set_layout_create_info, NULL, &vulkan_context.descriptor_set_layouts.shadow_map[SHADOW_MAP_UNIFORM_DESCRIPTOR_SET]));
		}
		{
			VkDescriptorSetLayoutBinding bindings[] = {
				uniform_buffer_layout_binding,
			};
			VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
				.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				.bindingCount = ArrayCount(bindings),
				.pBindings    = bindings,
			};
			VK_CHECK(vkCreateDescriptorSetLayout(vulkan_context.device, &descriptor_set_layout_create_info, NULL, &vulkan_context.descriptor_set_layouts.flat_color[FLAT_COLOR_UBO_DESCRIPTOR_SET_NUMBER]));
		}
	}

	// Command pool. @TODO STOP USING THIS POOL.
	{
		VkCommandPoolCreateInfo command_pool_create_info = {};
		command_pool_create_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		command_pool_create_info.queueFamilyIndex = vulkan_context.graphics_queue_family;
		command_pool_create_info.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		VK_CHECK(vkCreateCommandPool(vulkan_context.device, &command_pool_create_info, NULL, &vulkan_context.command_pool));
	}

	// Create shaders.
	{
		const char *textured_static_shaders[] = {"build/pbr_vertex.spirv", "build/pbr_fragment.spirv"};
		const char *shadow_map_static_shaders[] = {"build/shadow_map_vertex.spirv", "build/shadow_map_fragment.spirv"};
		const char *flat_color_shaders[] = {"build/flat_color_vertex.spirv", "build/flat_color_fragment.spirv"};
		// @TODO: Generate shader table.
		Create_Shader_Request requests[] = {
			{&vulkan_context.shaders[TEXTURED_STATIC_SHADER], textured_static_shaders, ArrayCount(textured_static_shaders)},
			{&vulkan_context.shaders[SHADOW_MAP_STATIC_SHADER], shadow_map_static_shaders, ArrayCount(shadow_map_static_shaders)},
			{&vulkan_context.shaders[FLAT_COLOR_SHADER], flat_color_shaders, ArrayCount(flat_color_shaders)},
		};
		for (s32 i = 0; i < ArrayCount(requests); i++) {
			assert(requests[i].shader_module_count < MAX_SHADER_MODULES);
			requests[i].shaders->module_count = requests[i].shader_module_count;
			requests[i].shaders->modules = (Shader_Module *)malloc(sizeof(Shader_Module) * requests[i].shader_module_count); // @TODO: allocate_array(&game_state->permanant_arena, Shader_Module, requests[i].shader_module_count);

			for (s32 j = 0; j < requests[i].shader_module_count; j++) {
				// @TODO: Subarena.
				String_Result spirv = Read_Entire_File(requests[i].paths[j], temporary_arena);
				Assert(spirv.data);

				VkShaderModuleCreateInfo shader_module_create_info = {};
				shader_module_create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				shader_module_create_info.codeSize = spirv.length;
				shader_module_create_info.pCode    = (u32 *)spirv.data;

				VK_CHECK(vkCreateShaderModule(vulkan_context.device, &shader_module_create_info, NULL, &requests[i].shaders->modules[j].module));

				if (find_substring(requests[i].paths[j], "vertex.spirv") != NULL) {
					requests[i].shaders->modules[j].stage = VK_SHADER_STAGE_VERTEX_BIT;
				} else if (find_substring(requests[i].paths[j], "fragment.spirv") != NULL) {
					requests[i].shaders->modules[j].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
				} else {
					_abort("Unknown shader type: %s", requests[i].paths[j]);
				}
			}
		}
	}

	create_vulkan_display_objects(temporary_arena);

	// GPU vertex/index/uniform resources.
	{
		VkBufferCreateInfo buffer_create_info = {};
		buffer_create_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.size        = VULKAN_GPU_ALLOCATION_SIZE;
		buffer_create_info.usage       = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_CHECK(vkCreateBuffer(vulkan_context.device, &buffer_create_info, NULL, &vulkan_context.gpu_buffer));

		VkMemoryRequirements memory_requirements;
		vkGetBufferMemoryRequirements(vulkan_context.device, vulkan_context.gpu_buffer, &memory_requirements);

		allocate_vulkan_memory(&vulkan_context.gpu_memory,  memory_requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VK_CHECK(vkBindBufferMemory(vulkan_context.device, vulkan_context.gpu_buffer, vulkan_context.gpu_memory, 0));
	}

	// Staging resources.
	{
		VkBufferCreateInfo buffer_create_info = {};
		buffer_create_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.size        = VULKAN_SHARED_ALLOCATION_SIZE;
		buffer_create_info.usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_CHECK(vkCreateBuffer(vulkan_context.device, &buffer_create_info, NULL, &vulkan_context.staging_buffer));

		VkMemoryRequirements memory_requirements;
		vkGetBufferMemoryRequirements(vulkan_context.device, vulkan_context.staging_buffer, &memory_requirements);

		allocate_vulkan_memory(&vulkan_context.shared_memory, memory_requirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT  | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT); // @TODO: Get rid of VK_MEMORY_PROPERTY_HOST_COHERENT_BIT and use vkFlushMappedMemoryRanges manually.
		VK_CHECK(vkBindBufferMemory(vulkan_context.device, vulkan_context.staging_buffer, vulkan_context.shared_memory, 0));
	}

	// Image memory.
	{
		VkMemoryRequirements memory_requirements = {
			.size = VULKAN_IMAGE_ALLOCATION_SIZE,
			.alignment = 0,
			.memoryTypeBits = 130, // @TODO: Some better way to get the memory type required for image memory?
		};
		allocate_vulkan_memory(&vulkan_context.image_memory, memory_requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	}

	// Semaphore.
	{
		VkSemaphoreCreateInfo semaphore_create_info = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		};
        VkFenceCreateInfo fence_create_info = {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT,
        };
        for (u32 i = 0; i < GPU_MAX_FRAMES_IN_FLIGHT; i++) {
			VK_CHECK(vkCreateSemaphore(vulkan_context.device, &semaphore_create_info, NULL, &vulkan_context.image_available_semaphores[i]));
			VK_CHECK(vkCreateSemaphore(vulkan_context.device, &semaphore_create_info, NULL, &vulkan_context.render_finished_semaphores[i]));
			VK_CHECK(vkCreateFence(vulkan_context.device, &fence_create_info, NULL, &vulkan_context.inFlightFences[i]));
        }
        fence_create_info.flags = 0;
        for (u32 i = 0; i < GPU_MAX_FRAMES_IN_FLIGHT; i++) {
			VK_CHECK(vkCreateFence(vulkan_context.device, &fence_create_info, NULL, &vulkan_context.assetFences[i]));
        }
	}

	// Uniform memory info.
	{
		vulkan_context.buffer_offsets.instance_memory_segment      = VULKAN_UNIFORM_MEMORY_SEGMENT_OFFSET + VULKAN_UNIFORM_MEMORY_SEGMENT_SIZE;
		vulkan_context.sizes.scene.aligned_dynamic_ubo             = align_to(sizeof(Dynamic_Scene_UBO), vulkan_context.minimum_uniform_buffer_offset_alignment);
		vulkan_context.sizes.scene.dynamic_uniform_buffer          = (TEST_INSTANCES * vulkan_context.sizes.scene.aligned_dynamic_ubo);
		u32 aligned_scene_uniform_buffer_object_size               = align_to(sizeof(Scene_UBO), vulkan_context.minimum_uniform_buffer_offset_alignment);
		u32 total_scene_uniform_buffer_size                        = vulkan_context.num_swapchain_images * aligned_scene_uniform_buffer_object_size;
		for (u32 i = 0; i < vulkan_context.num_swapchain_images; i++) {
			vulkan_context.buffer_offsets.scene.uniform[i]         = VULKAN_UNIFORM_MEMORY_SEGMENT_OFFSET + (i * aligned_scene_uniform_buffer_object_size);
			vulkan_context.buffer_offsets.scene.dynamic_uniform[i] = VULKAN_UNIFORM_MEMORY_SEGMENT_OFFSET + total_scene_uniform_buffer_size + (i * vulkan_context.sizes.scene.dynamic_uniform_buffer);
		}
		vulkan_context.buffer_offsets.scene.materials    = VULKAN_UNIFORM_MEMORY_SEGMENT_OFFSET + total_scene_uniform_buffer_size + (vulkan_context.num_swapchain_images * vulkan_context.sizes.scene.dynamic_uniform_buffer);
		vulkan_context.buffer_offsets.shadow_map.uniform = align_to(vulkan_context.buffer_offsets.scene.materials + (VULKAN_MAX_MATERIALS * sizeof(Vulkan_Material)), vulkan_context.minimum_uniform_buffer_offset_alignment);
		vulkan_context.buffer_offsets.flat_color.uniform = align_to(vulkan_context.buffer_offsets.shadow_map.uniform + sizeof(Shadow_Map_UBO), vulkan_context.minimum_uniform_buffer_offset_alignment);
	}

	// Descriptor pool.
	{
		// @TODO: Figure out what values these are supposed to have.
		VkDescriptorPoolSize pool_sizes[] = {
			{
				.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.descriptorCount = vulkan_context.num_swapchain_images + 60,
			},
			{
				.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
				.descriptorCount = vulkan_context.num_swapchain_images + 60,
			},
			{
				.type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.descriptorCount = vulkan_context.num_swapchain_images + 60,
			},
		};
		VkDescriptorPoolCreateInfo pool_info = {
			.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.poolSizeCount = ArrayCount(pool_sizes),
			.pPoolSizes    = pool_sizes,
			.maxSets       = vulkan_context.num_swapchain_images + 60,
			.flags         = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT,
		};
		VK_CHECK(vkCreateDescriptorPool(vulkan_context.device, &pool_info, NULL, &vulkan_context.descriptor_pool));
	}

	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;

	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	VK_CHECK(vkCreateSampler(vulkan_context.device, &samplerInfo, NULL, &vulkan_context.textureSampler));

	create_vulkan_command_buffers();

	// Descriptor set.
	{
		const u32 max_set_count = 100; // @TODO: Real value?
		VkDescriptorSet **sets = allocate_array(temporary_arena, VkDescriptorSet *, max_set_count);
		VkDescriptorSetLayout *layouts = allocate_array(temporary_arena, VkDescriptorSetLayout, max_set_count);
		u32 set_count = 0, layout_count = 0;

		sets[set_count++]       = &vulkan_context.descriptor_sets.scene.sampler;
		layouts[layout_count++] = vulkan_context.descriptor_set_layouts.scene[SCENE_SAMPLER_DESCRIPTOR_SET];

		sets[set_count++]       = &vulkan_context.descriptor_sets.scene.texture;
		layouts[layout_count++] = vulkan_context.descriptor_set_layouts.scene[SCENE_TEXTURE_DESCRIPTOR_SET];

		sets[set_count++]       = &vulkan_context.descriptor_sets.shadow_map.uniform;
		layouts[layout_count++] = vulkan_context.descriptor_set_layouts.shadow_map[SHADOW_MAP_UNIFORM_DESCRIPTOR_SET];

		sets[set_count++]       = &vulkan_context.descriptor_sets.flat_color.uniform;
		layouts[layout_count++] = vulkan_context.descriptor_set_layouts.flat_color[FLAT_COLOR_UBO_DESCRIPTOR_SET_NUMBER];

		sets[set_count++]       = &vulkan_context.descriptor_sets.scene.materials;
		layouts[layout_count++] = vulkan_context.descriptor_set_layouts.scene[SCENE_MATERIAL_DESCRIPTOR_SET];

		for (s32 i = 0; i < vulkan_context.num_swapchain_images; i++) {
			// We need a variable number of scene matrix descriptor sets because the matrix uniforms are stored in dynamic uniform buffers which have a maximum size.
			// If we want to store more matrix data than the maximum size, we may need more than one desciptor set.
			//vulkan_context.descriptor_sets.scene.dynamic_uniform[i]                = allocate_array(&game_state->permanent_arena, VkDescriptorSet, vulkan_context.scene_dynamic_uniform_buffer_count);
			//vulkan_context.descriptor_sets.scene.dynamic_uniform_buffer_offsets[i] = allocate_array(&game_state->permanent_arena, u32, vulkan_context.scene_dynamic_uniform_buffer_count);

			sets[set_count++]       = &vulkan_context.descriptor_sets.scene.uniform[i];
			layouts[layout_count++] = vulkan_context.descriptor_set_layouts.scene[SCENE_UNIFORM_DESCRIPTOR_SET];

			//for (u32 j = 0; j < vulkan_context.scene_dynamic_uniform_buffer_count; j++) {
				sets[set_count++]       = &vulkan_context.descriptor_sets.scene.dynamic_uniform[i];
				layouts[layout_count++] = vulkan_context.descriptor_set_layouts.scene[SCENE_DYNAMIC_UNIFORM_DESCRIPTOR_SET];
			//}
		}
		Assert(set_count < max_set_count);
		VkDescriptorSet *results = allocate_array(temporary_arena, VkDescriptorSet, set_count);
		VkDescriptorSetAllocateInfo allocate_info = {
			.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool     = vulkan_context.descriptor_pool,
			.descriptorSetCount = set_count,
			.pSetLayouts        = layouts,
		};
		VK_CHECK(vkAllocateDescriptorSets(vulkan_context.device, &allocate_info, results));
		for (u32 j = 0; j < set_count; j++) {
			*sets[j] = results[j];
		}

		VkWriteDescriptorSet *descriptor_writes = allocate_array(temporary_arena, VkWriteDescriptorSet, max_set_count);
		u32 write_count = 0;
		descriptor_writes[write_count++] = (VkWriteDescriptorSet){
			.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet          = vulkan_context.descriptor_sets.scene.materials,
			.dstBinding      = 5,
			.dstArrayElement = 0,
			.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.pBufferInfo     = &(VkDescriptorBufferInfo){
				.buffer = vulkan_context.gpu_buffer,
				.offset = vulkan_context.buffer_offsets.scene.materials,
				.range  = VULKAN_MAX_MATERIALS * sizeof(Vulkan_Material),
			},
		};
		descriptor_writes[write_count++] = (VkWriteDescriptorSet){
			.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet          = vulkan_context.descriptor_sets.scene.sampler,
			.dstBinding      = 2,
			.dstArrayElement = 0,
			.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER,
			.descriptorCount = 1,
			.pImageInfo      = &(VkDescriptorImageInfo){
				.sampler = vulkan_context.textureSampler,
			},
		};
		descriptor_writes[write_count++] = (VkWriteDescriptorSet){
			.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet          = vulkan_context.descriptor_sets.scene.sampler,
			.dstBinding      = 3,
			.dstArrayElement = 0,
			.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.pImageInfo      = &(VkDescriptorImageInfo){
				.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
				.imageView   = vulkan_context.framebuffer_attachments.shadow_map_depth.image_view,
				.sampler     = vulkan_context.samplers.shadow_map_depth,
			},
		};
		descriptor_writes[write_count++] = (VkWriteDescriptorSet){
			.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet          = vulkan_context.descriptor_sets.shadow_map.uniform,
			.dstBinding      = 0,
			.dstArrayElement = 0,
			.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.pBufferInfo     = &(VkDescriptorBufferInfo){
				.buffer = vulkan_context.gpu_buffer,
				.offset = vulkan_context.buffer_offsets.shadow_map.uniform,
				.range  = sizeof(Shadow_Map_UBO),
			},
		};
		descriptor_writes[write_count++] = (VkWriteDescriptorSet){
			.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet          = vulkan_context.descriptor_sets.flat_color.uniform,
			.dstBinding      = 0,
			.dstArrayElement = 0,
			.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.pBufferInfo     = &(VkDescriptorBufferInfo){
				.buffer = vulkan_context.gpu_buffer,
				.offset = vulkan_context.buffer_offsets.flat_color.uniform,
				.range  = sizeof(Flat_Color_UBO),
			},
		};
		// @TODO: Clean this up!
		VkDescriptorBufferInfo *buffer_infos = allocate_array(temporary_arena, VkDescriptorBufferInfo, 2 * vulkan_context.num_swapchain_images);
		u32 buffer_info_count = 0;
		for (s32 i = 0; i < vulkan_context.num_swapchain_images; i++) {
			buffer_infos[buffer_info_count] = (VkDescriptorBufferInfo){
				.buffer = vulkan_context.gpu_buffer,
					.offset = vulkan_context.buffer_offsets.scene.uniform[i],
					.range  = sizeof(Scene_UBO),
			};
			descriptor_writes[write_count++] = (VkWriteDescriptorSet){
				.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet          = vulkan_context.descriptor_sets.scene.uniform[i],
				.dstBinding      = 0,
				.dstArrayElement = 0,
				.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.descriptorCount = 1,
				.pBufferInfo     = &buffer_infos[buffer_info_count],
			};
			buffer_info_count++;
			buffer_infos[buffer_info_count] = (VkDescriptorBufferInfo){
				.buffer = vulkan_context.gpu_buffer,
				.offset = vulkan_context.buffer_offsets.scene.dynamic_uniform[i],
				.range  = vulkan_context.sizes.scene.dynamic_uniform_buffer,
			};
			descriptor_writes[write_count++] = (VkWriteDescriptorSet){
				.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet          = vulkan_context.descriptor_sets.scene.dynamic_uniform[i],
				.dstBinding      = 1,
				.dstArrayElement = 0,
				.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
				.descriptorCount = 1,
				.pBufferInfo     = &buffer_infos[buffer_info_count],
			};
			buffer_info_count++;
		}
		vkUpdateDescriptorSets(vulkan_context.device, write_count, descriptor_writes, 0, NULL);
	}

	// Set the default texture.
	{
	/* @TODO
		load_texture(S("data/default_texture.png"));
		VkDescriptorImageInfo *image_infos = allocate_array(temporary_arena, VkDescriptorImageInfo, VULKAN_MAX_TEXTURES);
		for (u32 i = 0; i < VULKAN_MAX_TEXTURES; i++) {
			image_infos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			image_infos[i].imageView   = vulkan_context.textures.image_views[0]; // @TODO: Use some kind of special "invalid" texture.
			image_infos[i].sampler     = NULL;
		}
		VkWriteDescriptorSet textures_descriptor_write = {
			.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstBinding      = 4,
			.dstArrayElement = 0,
			.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			.descriptorCount = VULKAN_MAX_TEXTURES,
			.pBufferInfo     = 0,
			.dstSet          = vulkan_context.descriptor_sets.scene.texture,
			.pImageInfo      = image_infos,
		};
		vkUpdateDescriptorSets(vulkan_context.device, 1, &textures_descriptor_write, 0, NULL);
	*/
	}

	vulkan_context.arena = permanent_arena;

	// @TODO: Try to allocate more than one chunk at startup? Would need to back off if it failed.
	//vulkan_context.base_gpu_memory_chunk = allocate_struct(permanent_arena, Vulkan_Memory_Chunk);
	if (!initialize_vulkan_dynamic_memory_allocator(&vulkan_context._gpu_memory, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
		_abort("Failed to initialize Vulkan GPU memory allocator: not enough memory for base memory block");
	}
	//vulkan_context.base_shared_memory_chunk = allocate_struct(permanent_arena, Vulkan_Memory_Chunk);
	if (!initialize_vulkan_dynamic_memory_allocator(&vulkan_context._shared_memory, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT  | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
		_abort("Failed to initialize Vulkan shared memory allocator: not enough memory for base memory block");
	}
	if (!(vulkan_context._staging_memory = acquire_vulkan_memory(&vulkan_context._shared_memory, VULKAN_STAGING_MEMORY_SIZE))) {
		_abort("Failed to allocate Vulkan staging memory");
	}

	game_state->gpu_context.thread_local = malloc(game_state->jobs_context.worker_thread_count * sizeof(GPU_Thread_Local_Context));
	for (u32 i = 0; i < game_state->jobs_context.worker_thread_count; i++) {
		for (u32 j = 0; j < GPU_MAX_FRAMES_IN_FLIGHT; j++) {
			VkCommandPoolCreateInfo command_pool_create_info = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
				.queueFamilyIndex = vulkan_context.graphics_queue_family,
				//.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			};
			VK_CHECK(vkCreateCommandPool(vulkan_context.device, &command_pool_create_info, NULL, &game_state->gpu_context.thread_local[i].command_pools[j]));
		}
	}
#endif
	// @TODO: Try to use a seperate queue family for transfer operations so that it can be parallelized.
}

#if 0

void update_vulkan_uniforms(M4 scene_projection, Camera *camera, Mesh_Instance *meshes, u32 *visible_meshes, u32 visible_mesh_count, u32 swapchain_image_index) {
	// @TODO: Does all of this need to happen every frame? Probably not!
	// Shadow map.
	{
		M4 model = m4_identity();
		M4 view = look_at((V3){2.0f, 2.0f, 2.0f}, (V3){0.0f, 0.0f, 0.0f}, (V3){0.0f, 0.0f, 1.0f});
		M4 projection = orthographic_projection(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 20.0f);
		shadow_map_ubo.world_to_clip_space = multiply_m4(multiply_m4(projection, view), model);
		stage_vulkan_data(&shadow_map_ubo, sizeof(Shadow_Map_UBO));
		transfer_staged_vulkan_data(vulkan_context.buffer_offsets.shadow_map.uniform);
	}

	M4 scene_projection_view = multiply_m4(scene_projection, camera->view_matrix);

	// Scene.
	{
		static const M4 shadow_map_clip_space_bias = {{
			{0.5, 0.0, 0.0, 0.5},
			{0.0, 0.5, 0.0, 0.5},
			{0.0, 0.0, 1.0, 0.0},
			{0.0, 0.0, 0.0, 1.0},
		}};
		M4 model = m4_identity();
		for (s32 i = 0; i < visible_mesh_count; i++) {
			set_matrix_translation(&model, meshes[visible_meshes[i]].transform.translation);
			dynamic_scene_ubo[i] = (Dynamic_Scene_UBO){
				.model_to_world_space           = multiply_m4(scene_projection_view, model),
				.world_to_clip_space            = multiply_m4(scene_projection_view, model),
				.world_to_shadow_map_clip_space = multiply_m4(multiply_m4(shadow_map_clip_space_bias, shadow_map_ubo.world_to_clip_space), model),
			};
		}
	}

	Scene_UBO scene_ubo = {
		.camera_position = camera->position,
	};
	stage_vulkan_data(&scene_ubo, sizeof(scene_ubo));
	transfer_staged_vulkan_data(vulkan_context.buffer_offsets.scene.uniform[swapchain_image_index]);

	//for (s32 i = 0; i < TEST_INSTANCES; i++) {
	for (u32 i = 0; i < visible_mesh_count; i++) {
		stage_vulkan_data(&dynamic_scene_ubo[i], sizeof(dynamic_scene_ubo[i]));
		transfer_staged_vulkan_data(vulkan_context.buffer_offsets.scene.dynamic_uniform[swapchain_image_index] + (i * vulkan_context.sizes.scene.aligned_dynamic_ubo));
		//transfer_staged_vulkan_data(xxx + vulkan_context.scene_dynamic_uniform_buffer_offsets[vulkan_context.current_swapchain_image_index] + (i * vulkan_context.aligned_scene_uniform_buffer_object_size));
		//u32 x = dubo_offset + i * align_to(sizeof(Dynamic_Scene_UBO), vulkan_context.minimum_uniform_buffer_offset_alignment);
		//assert(x % vulkan_context.minimum_uniform_buffer_offset_alignment == 0);
		//transfer_staged_vulkan_data(vulkan_context.scene_dynamic_uniform_buffer_offset + (i * vulkan_context.aligned_scene_dynamic_uniform_buffer_object_size));
	}

	// @TODO: Materials should be in a linear array.
	// @TODO: Load all materials from directory on startup.
	// @TODO: Only update materials when they need to be updated.
	for (u32 i = 0; i < visible_mesh_count; i++) {
		// WRONG!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! need a submesh index, not the mesh index
		Vulkan_Material material = {
			.albedo_map            = meshes[visible_meshes[i]].asset->materials[0].albedo_map,
			.normal_map            = meshes[visible_meshes[i]].asset->materials[0].normal_map,
			.metallic_map          = meshes[visible_meshes[i]].asset->materials[0].metallic_map,
			.roughness_map         = meshes[visible_meshes[i]].asset->materials[0].roughness_map,
			.ambient_occlusion_map = meshes[visible_meshes[i]].asset->materials[0].ambient_occlusion_map,
		};
		stage_vulkan_data(&material, sizeof(material));
		transfer_staged_vulkan_data(vulkan_context.buffer_offsets.scene.materials + (i * sizeof(Vulkan_Material)));
	}

	M4 m = m4_identity();
	set_matrix_translation(&m, (V3){-0.398581, 0.000000, 1.266762});
	Flat_Color_UBO ubo = {
		.color = {1.0, 0.0f, 0.0f, 1.0f},
		.model_view_projection = scene_projection_view,
	};
	stage_vulkan_data(&ubo, sizeof(ubo));
	transfer_staged_vulkan_data(vulkan_context.buffer_offsets.flat_color.uniform);
}

void build_vulkan_command_buffer(Mesh_Instance *meshes, u32 *visible_meshes, u32 visible_mesh_count, Render_Context *render_context, u32 swapchain_image_index) {
	VkCommandBuffer command_buffer = vulkan_context.command_buffers[swapchain_image_index];
	vkResetCommandBuffer(command_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

	VkClearValue clear_color = {{{0.04f, 0.19f, 0.34f, 1.0f}}};
	VkClearValue clear_depth_stencil = {{{1.0f, 0.0f}}};
	VkClearValue clear_values[] = {
		clear_color,
		clear_depth_stencil,
	};
	VkBuffer vertex_buffers[] = {vulkan_context.gpu_buffer};
	VkDeviceSize offsets[] = {0};

	VkCommandBufferBeginInfo command_buffer_begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
	};
	VK_CHECK(vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info));

	/*
	// Shadow map.
	{
		VkRenderPassBeginInfo render_pass_begin_info = {
			.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.renderPass               = vulkan_context.render_passes.shadow_map,
			.framebuffer              = vulkan_context.xframebuffers.shadow_map,
			.renderArea.extent.width  = SHADOW_MAP_WIDTH,
			.renderArea.extent.height = SHADOW_MAP_HEIGHT,
			.clearValueCount          = 1,
			.pClearValues             = &clear_depth_stencil,
		};
		vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
		VkViewport viewport = {
			.x        = 0.0f,
			.y        = 0.0f,
			.width    = (f32)SHADOW_MAP_WIDTH,
			.height   = (f32)SHADOW_MAP_HEIGHT,
			.minDepth = 0.0f,
			.maxDepth = 1.0f,
		};
		vkCmdSetViewport(command_buffer, 0, 1, &viewport);
		VkRect2D scissor = {
			.offset = {0, 0},
			.extent = {SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT},
		};
		vkCmdSetScissor(command_buffer, 0, 1, &scissor);
		vkCmdSetDepthBias(command_buffer, SHADOW_MAP_CONSTANT_DEPTH_BIAS, 0.0f, SHADOW_MAP_SLOPE_DEPTH_BIAS); // Set depth bias, required to avoid shadow mapping artifacts.
		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan_context.pipelines.shadow_map_static);
		vkCmdBindVertexBuffers(command_buffer, VULKAN_VERTEX_BUFFER_BIND_ID, 1, &vulkan_context.gpu_buffer, &(VkDeviceSize){0});
		//vkCmdBindVertexBuffers(command_buffer, VULKAN_INSTANCE_BUFFER_BIND_ID, 1, &vulkan_context.gpu_buffer, &(VkDeviceSize){vulkan_context.buffer_offsets.instance_memory_segment});
		vkCmdBindIndexBuffer(command_buffer, vulkan_context.gpu_buffer, VULKAN_INDEX_MEMORY_SEGMENT_OFFSET, VK_INDEX_TYPE_UINT32);
		for (u32 i = 0; i < visible_mesh_count; i++) {
			u32 mesh_index = visible_meshes[i];
			u32 total_mesh_index_count = 0;
			for (u32 j = 0; j < submesh_counts[mesh_index]; j++) {
				vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan_context.pipeline_layouts.shadow_map, SHADOW_MAP_UNIFORM_DESCRIPTOR_SET, 1, &vulkan_context.descriptor_sets.shadow_map.uniform, 0, NULL);
				vkCmdDrawIndexed(command_buffer, meshes[mesh_index].submesh_index_counts[j], 1, total_mesh_index_count + meshes[mesh_index].first_index, meshes[mesh_index].vertex_offset, 0);
				total_mesh_index_count += meshes[mesh_index].submesh_index_counts[j];
			}
		}
		vkCmdEndRenderPass(command_buffer);
	}
	*/

	// Scene.
	{
		VkRenderPassBeginInfo render_pass_begin_info = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.renderPass = vulkan_context.render_pass,
			.framebuffer = vulkan_context.framebuffers[swapchain_image_index],
			.renderArea.offset = {0, 0},
			.renderArea.extent = vulkan_context.swapchain_image_extent,
			.clearValueCount = ArrayCount(clear_values),
			.pClearValues = clear_values,
		};
		vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = {
			.x = 0.0f,
			.y = 0.0f,
			.width = (f32)vulkan_context.swapchain_image_extent.width,
			.height = (f32)vulkan_context.swapchain_image_extent.height,
			.minDepth = 0.0f,
			.maxDepth = 1.0f,
		};
		vkCmdSetViewport(command_buffer, 0, 1, &viewport);

		VkRect2D scissor = {
			.offset = {0, 0},
			.extent = {vulkan_context.swapchain_image_extent.width, vulkan_context.swapchain_image_extent.height},
		};
		vkCmdSetScissor(command_buffer, 0, 1, &scissor);
		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan_context.pipelines.textured_static);
		// @TODO: Combine these vkCmdBindVertexBuffers calls.
		vkCmdBindVertexBuffers(command_buffer, VULKAN_INSTANCE_BUFFER_BIND_ID, 1, &vulkan_context.gpu_buffer, &(VkDeviceSize){vulkan_context.buffer_offsets.instance_memory_segment});
		// @TODO: Bind these all at the same time?
		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan_context.pipeline_layout, SCENE_UNIFORM_DESCRIPTOR_SET, 1, &vulkan_context.descriptor_sets.scene.uniform[swapchain_image_index], 0, NULL);
		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan_context.pipeline_layout, SCENE_MATERIAL_DESCRIPTOR_SET, 1, &vulkan_context.descriptor_sets.scene.materials, 0, NULL);
		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan_context.pipeline_layout, SCENE_SAMPLER_DESCRIPTOR_SET, 1, &vulkan_context.descriptor_sets.scene.sampler, 0, NULL);
		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan_context.pipeline_layout, SCENE_TEXTURE_DESCRIPTOR_SET, 1, &vulkan_context.descriptor_sets.scene.texture, 0, NULL);
		for (u32 i = 0; i < visible_mesh_count; i++) {
			u32 mesh_index = visible_meshes[i];
			vkCmdBindVertexBuffers(command_buffer, VULKAN_VERTEX_BUFFER_BIND_ID, 1, &meshes[mesh_index].asset->gpu_mesh.memory->buffer, (u64 *)&meshes[mesh_index].asset->gpu_mesh.memory->offset);
			vkCmdBindIndexBuffer(command_buffer, meshes[mesh_index].asset->gpu_mesh.memory->buffer, meshes[mesh_index].asset->gpu_mesh.memory->offset + meshes[mesh_index].asset->gpu_mesh.indices_offset, VK_INDEX_TYPE_UINT32);
			u32 total_mesh_index_count = 0;
			for (u32 j = 0; j < meshes[mesh_index].asset->submesh_count; j++) {
				u32 offset_inside_dynamic_uniform_buffer = i * vulkan_context.sizes.scene.aligned_dynamic_ubo;
				vkCmdBindDescriptorSets(command_buffer,
				                        VK_PIPELINE_BIND_POINT_GRAPHICS,
				                        vulkan_context.pipeline_layout,
				                        SCENE_DYNAMIC_UNIFORM_DESCRIPTOR_SET,
				                        1,
				                        &vulkan_context.descriptor_sets.scene.dynamic_uniform[swapchain_image_index],
				                        1,
				                        &offset_inside_dynamic_uniform_buffer);
				//vkCmdDrawIndexed(command_buffer, meshes[mesh_index].submesh_index_counts[j], 1, total_mesh_index_count + meshes[mesh_index].first_index, meshes[mesh_index].vertex_offset, i);
				vkCmdDrawIndexed(command_buffer, meshes[mesh_index].asset->submesh_index_counts[j], 1, total_mesh_index_count, 0, i);
				total_mesh_index_count += meshes[mesh_index].asset->submesh_index_counts[j];
			}
		}

		for (u32 i = 0; i < render_context->debug_render_object_count; i++) {
			V4 push_constant = render_context->debug_render_objects[i].color;
			vkCmdPushConstants(command_buffer, vulkan_context.pipeline_layouts.flat_color, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(V4), (void *)&push_constant);
			vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan_context.pipelines.flat_color);
			VkDeviceSize offsetss[] = {VULKAN_FRAME_VERTEX_MEMORY_SEGMENT_OFFSET + (vulkan_context.currentFrame * VULKAN_FRAME_VERTEX_MEMORY_SEGMENT_SIZE)};
			vkCmdBindVertexBuffers(command_buffer, 0, 1, &vulkan_context.staging_buffer, offsetss);
			vkCmdBindIndexBuffer(command_buffer, vulkan_context.staging_buffer, VULKAN_FRAME_INDEX_MEMORY_SEGMENT_OFFSET + (vulkan_context.currentFrame * VULKAN_FRAME_INDEX_MEMORY_SEGMENT_SIZE), VK_INDEX_TYPE_UINT32);
			vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan_context.pipeline_layouts.flat_color, FLAT_COLOR_UBO_DESCRIPTOR_SET_NUMBER, 1, &vulkan_context.descriptor_sets.flat_color.uniform, 0, NULL);
			vkCmdDrawIndexed(command_buffer, render_context->debug_render_objects[i].index_count, 1, render_context->debug_render_objects[i].first_index, render_context->debug_render_objects[i].vertex_offset, 0);
			//vkCmdDraw(command_buffer, render_context->debug_render_objects[i].vertex_count, 1, total_vertex_count, 0);
		}

		vkCmdEndRenderPass(command_buffer);
	}

	VK_CHECK(vkEndCommandBuffer(command_buffer));
}

// @TODO: A way to wait for the next frame without blocking? Or maybe we wait to see if there is any more work and then wait?
u32 GPU_Wait_For_Available_Frame(GPU_Context *context) {
	vulkan_context.currentFrame = vulkan_context.nextFrame;
	vulkan_context.nextFrame = (vulkan_context.nextFrame + 1) % GPU_MAX_FRAMES_IN_FLIGHT;
	vkWaitForFences(vulkan_context.device, 1, &vulkan_context.inFlightFences[vulkan_context.currentFrame], 1, UINT64_MAX);
	vkResetFences(vulkan_context.device, 1, &vulkan_context.inFlightFences[vulkan_context.currentFrame]);
	vkResetCommandPool(vulkan_context.device, context->thread_local[thread_index].command_pools[vulkan_context.currentFrame], 0);
	u32 swapchain_image_index = 0;
	VK_CHECK(vkAcquireNextImageKHR(vulkan_context.device, vulkan_context.swapchain, UINT64_MAX, vulkan_context.image_available_semaphores[vulkan_context.currentFrame], NULL, &swapchain_image_index));
	return swapchain_image_index;
}

void GPU_Transfer_Data(void *source, GPU_Memory_Location destination, u32 size) {
	//Platform_Lock_Mutex(&vulkan_context.mutex);
	stage_vulkan_data(source, size);
	transfer_staged_vulkan_data(destination.offset);
	//Platform_Unlock_Mutex(&vulkan_context.mutex);
	//transfer_staged_vulkan_data(vulkan_context.buffer_offsets.instance_memory_segment + offset_inside_instance_memory_segment);
}

GPU_Memory_Allocation *GPU_Acquire_Memory(GPU_Memory_Allocator *allocator, u32 size) {
	for (u32 i = 0; i < allocator->active_block->freed_allocation_count; i++) {
		Assert(0); // @TODO
	}
	GPU_Memory_Allocation *new_allocation = &allocator->active_block->active_allocations[allocator->active_block->active_allocation_count];
	*new_allocation = (GPU_Memory_Allocation){
		.block          = allocator->active_block,
		.size           = size,
		.mapped_pointer = allocator->active_block->mapped_pointer,
		.buffer         = allocator->active_block->buffer,
		.offset         = allocator->active_block->frontier,
	};
	allocator->active_block->active_allocation_count++;
	Assert(allocator->active_block->active_allocation_count < VULKAN_MAX_MEMORY_ALLOCATIONS_PER_BLOCK);
	allocator->active_block->frontier += size;
	Assert(allocator->active_block->frontier < VULKAN_MEMORY_BLOCK_SIZE);
	return new_allocation;
}

void GPU_Flush_Asset_Uploads() {
/*
	u32 count = vulkan_context.asset_upload_command_buffers.ring_buffer.write_index - vulkan_context.asset_upload_command_buffers.ring_buffer.read_index; // @TODO BROKEN
	if (count == 0) {
		return;
	}
	Platform_Lock_Mutex(&vulkan_context.mutex);
	VkCommandBuffer *command_buffer;
	VkCommandBuffer buffers[100]; // @TODO
	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = count,
		.pCommandBuffers = buffers,
	};
	u32 i = 0;
	while (vulkan_context.asset_upload_command_buffers.ring_buffer.read_index != vulkan_context.asset_upload_command_buffers.ring_buffer.write_index) {
		// @TODO: Use a double buffer array?
		Atomic_Read_From_Ring_Buffer(vulkan_context.asset_upload_command_buffers, command_buffer);
		buffers[i++] = *command_buffer;
	}
	VK_CHECK(vkQueueSubmit(vulkan_context.graphics_queue, 1, &submit_info, VK_NULL_HANDLE));
	Platform_Unlock_Mutex(&vulkan_context.mutex);
*/
}

void vulkan_submit(Camera *camera, Mesh_Instance *meshes, u32 *visible_meshes, u32 visible_mesh_count, Render_Context *render_context, u32 swapchain_image_index) {
	Platform_Lock_Mutex(&vulkan_context.mutex);

	//update_vulkan_uniforms(render_context->scene_projection, camera, meshes, visible_meshes, visible_mesh_count, swapchain_image_index);
	//vkWaitForFences(vulkan_context.device, 1, &the_fence, 1, UINT64_MAX);
	//build_vulkan_command_buffer(meshes, visible_meshes, visible_mesh_count, render_context, swapchain_image_index);

	VkSemaphore wait_semaphores[] = {vulkan_context.image_available_semaphores[vulkan_context.currentFrame]};
	VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSemaphore signal_semaphores[] = {vulkan_context.render_finished_semaphores[vulkan_context.currentFrame]};

	VkSubmitInfo submit_info = {
		.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount   = ArrayCount(wait_semaphores),
		.pWaitSemaphores      = wait_semaphores,
		.pWaitDstStageMask    = wait_stages,
		.commandBufferCount   = 0,
		.pCommandBuffers      = &vulkan_context.command_buffers[swapchain_image_index],
		.signalSemaphoreCount = ArrayCount(signal_semaphores),
		.pSignalSemaphores    = signal_semaphores,
	};
	VK_CHECK(vkQueueSubmit(vulkan_context.graphics_queue, 1, &submit_info, vulkan_context.inFlightFences[vulkan_context.currentFrame]));

/* THIS STUFF NEEDS TO HAPPEN TO PROPERLY DIFFERENT GRAPHICS AND PRESENT QUEUES
    if (demo->separate_present_queue) {
        // If we are using separate queues, change image ownership to the
        // present queue before presenting, waiting for the draw complete
        // semaphore and signalling the ownership released semaphore when finished
        VkFence nullFence = VK_NULL_HANDLE;
        pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &demo->draw_complete_semaphores[demo->frame_index];
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &demo->swapchain_image_resources[demo->current_buffer].graphics_to_present_cmd;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &demo->image_ownership_semaphores[demo->frame_index];
        err = vkQueueSubmit(demo->present_queue, 1, &submit_info, nullFence);
        assert(!err);
    }

    // If we are using separate queues we have to wait for image ownership,
    // otherwise wait for draw complete
    VkPresentInfoKHR present = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = NULL,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = (demo->separate_present_queue) ? &demo->image_ownership_semaphores[demo->frame_index]
                                                          : &demo->draw_complete_semaphores[demo->frame_index],
        .swapchainCount = 1,
        .pSwapchains = &demo->swapchain,
        .pImageIndices = &demo->current_buffer,
    };
*/

	VkSwapchainKHR swapchains[] = {vulkan_context.swapchain};
	VkPresentInfoKHR present_info = {
		.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = ArrayCount(signal_semaphores),
		.pWaitSemaphores    = signal_semaphores,
		.swapchainCount     = ArrayCount(swapchains),
		.pSwapchains        = swapchains,
		.pImageIndices      = &swapchain_image_index,
	};
	VK_CHECK(vkQueuePresentKHR(vulkan_context.present_queue, &present_info));

	vulkan_context.debug_vertex_memory_bytes_used = 0;
	vulkan_context.debug_index_memory_bytes_used = 0;
	vulkan_context.debug_vertex_count = 0;
	vulkan_context.debug_index_count = 0;

	Platform_Unlock_Mutex(&vulkan_context.mutex);
}

void Cleanup_Renderer() {
	vkDeviceWaitIdle(vulkan_context.device);

	if (debug) {
		vkDestroyDebugUtilsMessengerEXT(vulkan_context.instance, vulkan_context.debug_messenger, NULL);
	}

	for (s32 i = 0; i < GPU_SHADER_COUNT; i++) {
		for (s32 j = 0; j < vulkan_context.shaders[i].module_count; j++) {
			vkDestroyShaderModule(vulkan_context.device, vulkan_context.shaders[i].modules[j].module, NULL);
		}
	}

	for (s32 i = 0; i < GPU_MAX_FRAMES_IN_FLIGHT; i++) {
        vkWaitForFences(vulkan_context.device, 1, &vulkan_context.inFlightFences[i], VK_TRUE, UINT64_MAX);
		vkDestroyFence(vulkan_context.device, vulkan_context.inFlightFences[i], NULL);
		vkDestroySemaphore(vulkan_context.device, vulkan_context.image_available_semaphores[i], NULL);
		vkDestroySemaphore(vulkan_context.device, vulkan_context.render_finished_semaphores[i], NULL);
	}
	for (s32 i = 0; i < vulkan_context.num_swapchain_images; i++) {
		vkDestroyFramebuffer(vulkan_context.device, vulkan_context.framebuffers[i], NULL);
	}
	//vkDestroyPipeline(vulkan_context.device, vulkan_context.pipeline, NULL);
	vkDestroyRenderPass(vulkan_context.device, vulkan_context.render_pass, NULL);
	vkDestroyPipelineLayout(vulkan_context.device, vulkan_context.pipeline_layout, NULL);
	for (s32 i = 0; i < vulkan_context.num_swapchain_images; i++) {
		vkDestroyImageView(vulkan_context.device, vulkan_context.swapchain_image_views[i], NULL);
		//vkDestroyBuffer(vulkan_context.device, vulkan_context.uniform_buffers[i], NULL);
		//vkFreeMemory(vulkan_context.device, vulkan_context.uniform_buffers_memory[i], NULL);
	}
	vkDestroySwapchainKHR(vulkan_context.device, vulkan_context.swapchain, NULL);

	vkDestroySampler(vulkan_context.device, vulkan_context.textureSampler, NULL);
	//vkDestroyImageView(vulkan_context.device, vulkan_context.textureImageView, NULL);

	//vkDestroyImage(vulkan_context.device, vulkan_context.textureImage, NULL);
	vkFreeMemory(vulkan_context.device, vulkan_context.image_memory, NULL);

	vkDestroyDescriptorPool(vulkan_context.device, vulkan_context.descriptor_pool, NULL);
	//vkDestroyDescriptorSetLayout(vulkan_context.device, vulkan_context.descriptor_set_layout, NULL);

	//vkDestroyBuffer(vulkan_context.device, vulkan_context.vertex_buffer, NULL);
	//vkFreeMemory(vulkan_context.device, vulkan_context.vertex_buffer_memory, NULL);

	//vkDestroyBuffer(vulkan_context.device, vulkan_context.index_buffer, NULL);
	vkFreeMemory(vulkan_context.device, vulkan_context.gpu_memory, NULL);
	vkFreeMemory(vulkan_context.device, vulkan_context.shared_memory, NULL);

	vkDestroyCommandPool(vulkan_context.device, vulkan_context.command_pool, NULL);

	vkDeviceWaitIdle(vulkan_context.device);

	vkDestroyDevice(vulkan_context.device, NULL);
	vkDestroySurfaceKHR(vulkan_context.instance, vulkan_context.surface, NULL);

	Platform_Cleanup_Display();

	vkDestroyInstance(vulkan_context.instance, NULL); // On X11, the Vulkan instance must be destroyed after the display resources are destroyed.
}

#endif

#endif
