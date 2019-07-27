#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_xlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// @TODO: Make sure that all failable Vulkan calls are VK_CHECK'd.

#define MAX_FRAMES_IN_FLIGHT 2

#define SHADOW_MAP_WIDTH 1024
#define SHADOW_MAP_HEIGHT 1024
// Depth bias and slope are used to avoid shadowing artifacts.
// Constant depth bias factor, which is always applied.
#define CONSTANT_DEPTH_BIAS 0.25
// Slope depth bias factor, which is applied depending on the polygon's slope.
#define SLOPE_DEPTH_BIAS 1.25
#define SHADOW_MAP_DEPTH_FORMAT VK_FORMAT_D16_UNORM
#define SHADOW_MAP_COLOR_FORMAT VK_FORMAT_R8G8B8A8_UNORM
#define SHADOW_MAP_FILTER VK_FILTER_LINEAR

struct Shader_Module {
	VkShaderModule module;
	VkShaderStageFlagBits stage;
};

struct Shaders {
	u32 module_count;
	Shader_Module *modules;
};

struct Framebuffer_Attachment {
	VkImage image;
	VkImageView image_view;
	VkDeviceMemory memory;
};

#define VULKAN_GPU_ALLOCATION_SIZE MEGABYTE(100)
#define VULKAN_SHARED_ALLOCATION_SIZE MEGABYTE(100)
#define VULKAN_IMAGE_ALLOCATION_SIZE MEGABYTE(100)

#define VULKAN_VERTEX_INDEX_SEGMENT_SIZE ((u32)(VULKAN_GPU_ALLOCATION_SIZE * .75))
#define VULKAN_UNIFORM_SEGMENT_SIZE ((u32)(VULKAN_GPU_ALLOCATION_SIZE * .25))

#define VULKAN_VERTEX_INDEX_SEGMENT_OFFSET (0)
#define VULKAN_UNIFORM_SEGMENT_OFFSET (VULKAN_VERTEX_INDEX_SEGMENT_OFFSET + VULKAN_VERTEX_INDEX_SEGMENT_SIZE)

#define VULKAN_MAX_TEXTURE_COUNT (1000)

struct Vulkan_Context {
	VkDebugUtilsMessengerEXT debug_messenger;
	VkInstance               instance;
	VkPhysicalDevice         physical_device;
	VkDevice                 device;
	VkSurfaceKHR             surface;
	VkSurfaceFormatKHR       surface_format;
	VkPresentModeKHR         present_mode;
	u32                      graphics_queue_family;
	u32                      present_queue_family;
	VkQueue                  graphics_queue;
	VkQueue                  present_queue;
	VkSwapchainKHR           swapchain;
	VkExtent2D               swapchain_image_extent;
	u32                      num_swapchain_images;
	VkImageView              swapchain_image_views[3];
	VkFramebuffer            framebuffers[3];
	VkRenderPass             render_pass;
	//VkPipeline               pipeline;
	VkPipelineLayout         pipeline_layout;
	VkDescriptorSetLayout    descriptor_set_layout;
	VkDescriptorPool         descriptor_pool;
	VkDescriptorSet          descriptor_sets[3];
	VkCommandPool            command_pool;
	VkCommandBuffer          command_buffers[3];
	//VkBuffer                 vertex_buffer;
	//VkDeviceMemory           vertex_buffer_memory;
	//VkBuffer                 index_buffer;
	//VkDeviceMemory           index_buffer_memory;
	//VkBuffer                 uniform_buffers[3];
	//VkDeviceMemory           uniform_buffers_memory[3];
	VkSemaphore              image_available_semaphores[MAX_FRAMES_IN_FLIGHT];
	VkSemaphore              render_finished_semaphores[MAX_FRAMES_IN_FLIGHT];
	VkFence                  inFlightFences[MAX_FRAMES_IN_FLIGHT];
	u32                      currentFrame;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	VkDeviceMemory shared_memory;
	VkDeviceMemory gpu_memory;
	VkDeviceMemory image_memory;

	VkBuffer gpu_buffer;
	VkBuffer staging_buffer;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	VkDescriptorImageInfo image_infos[VULKAN_MAX_TEXTURE_COUNT];

	struct {
		VkDescriptorSet shadow_map;
	} xdescriptor_sets;

	struct {
		VkPipelineLayout shadow_map;
	} pipeline_layouts;

	struct {
		VkPipeline textured_static;
		VkPipeline shadow_map_static;
	} pipelines;

	struct {
		VkRenderPass shadow_map;
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

	u32 vertex_count;
	u32 index_count;
	u32 vertex_index_offset;
	VkDeviceSize minimum_uniform_buffer_offset_alignment;

	Shaders shaders[SHADER_COUNT];
} vulkan_context;

#define VK_CHECK(x)\
	do {\
		auto result = (x);\
		if (result != VK_SUCCESS) _abort("VK_CHECK failed on '%s': %s", #x, vk_result_to_string(result));\
	} while (0)

#define VK_EXPORTED_FUNCTION(name)\
	PFN_##name name = NULL;
#define VK_GLOBAL_FUNCTION(name)\
	PFN_##name name = NULL;
#define VK_INSTANCE_FUNCTION(name)\
	PFN_##name name = NULL;
#define VK_DEVICE_FUNCTION(name)\
	PFN_##name name = NULL;

#include "vulkan_functions.h"

#undef VK_EXPORTED_FUNCTION
#undef VK_GLOBAL_FUNCTION
#undef VK_INSTANCE_FUNCTION
#undef VK_DEVICE_FUNCTION

const char *vk_result_to_string(VkResult result) {
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

u32 vulkan_debug_message_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data) {
	Log_Type log_type;
	const char *severity_string;
	switch (severity) {
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: {
		log_type = STANDARD_LOG;
		severity_string = "INFO";
	} break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
		log_type = MINOR_ERROR_LOG;
		severity_string = "WARNING";
	} break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
		log_type = CRITICAL_ERROR_LOG;
		severity_string = "ERROR";
	} break;
	default: {
		log_type = STANDARD_LOG;
	};
	}

	const char *type_string;
	switch(type) {
	case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: {
		type_string = "General";
	} break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: {
		type_string = "Validation";
	} break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: {
		type_string = "Performance";
	} break;
	default: {
		type_string = "Unknown";
	};
	}

	log_print(log_type, "%s: %s: %s", severity_string, type_string, callback_data->pMessage);
    return false;
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

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
                VkCommandBuffer commandBuffer = beginSingleTimeCommands();

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
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        endSingleTimeCommands(commandBuffer);
    }

void allocate_vulkan_memory(VkDeviceMemory *memory, VkMemoryRequirements memory_requirements, VkMemoryPropertyFlags desired_memory_properties) {
	VkMemoryAllocateInfo memory_allocate_info = {};
	memory_allocate_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_allocate_info.allocationSize  = memory_requirements.size;

	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(vulkan_context.physical_device, &memory_properties);
	s32 selected_memory_type_index = -1;
	for (s32 i = 0; i < memory_properties.memoryTypeCount; i++) {
		if ((memory_requirements.memoryTypeBits & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & desired_memory_properties) == desired_memory_properties) {
			selected_memory_type_index = i;
			break;
		}
	}
	if (selected_memory_type_index < 0) {
		_abort("Failed to find suitable GPU memory type");
	}

	memory_allocate_info.memoryTypeIndex = selected_memory_type_index;

	VK_CHECK(vkAllocateMemory(vulkan_context.device, &memory_allocate_info, NULL, memory)); // @TODO: Custom allocator.
}

void create_vulkan_pipelines(const char *vertex_shader_path, const char *fragment_shader_path, VkPipeline *pipeline) {
}

struct Create_Graphics_Pipeline_Request {
	VkPipeline *pipeline;
	VkPipelineLayout pipeline_layout;
	VkRenderPass render_pass;
	Shaders *shaders;
};

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
		//debug_print("\tSwap extent %u x %u\n", sc.extent.width, sc.extent.height);

		u32 num_requested_swapchain_images = surface_capabilities.minImageCount + 1;
		if (surface_capabilities.maxImageCount > 0 && num_requested_swapchain_images > surface_capabilities.maxImageCount) {
			num_requested_swapchain_images = surface_capabilities.maxImageCount;
		}
		//debug_print("\tMax swap images: %u\n\tMin swap images: %u\n", surface_capabilities.maxImageCount, surface_capabilities.minImageCount);
		//debug_print("\tRequested swap image count: %u\n", num_requested_swapchain_images);

		VkSwapchainCreateInfoKHR swapchain_create_info = {};
		swapchain_create_info.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchain_create_info.surface          = vulkan_context.surface;
		swapchain_create_info.minImageCount    = num_requested_swapchain_images;
		swapchain_create_info.imageFormat      = vulkan_context.surface_format.format;
		swapchain_create_info.imageColorSpace  = vulkan_context.surface_format.colorSpace;
		swapchain_create_info.imageExtent      = vulkan_context.swapchain_image_extent;
		swapchain_create_info.imageArrayLayers = 1;
		swapchain_create_info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchain_create_info.preTransform     = surface_capabilities.currentTransform;
		swapchain_create_info.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchain_create_info.presentMode      = vulkan_context.present_mode;
		swapchain_create_info.clipped          = true;
		swapchain_create_info.oldSwapchain     = NULL;

		if (vulkan_context.graphics_queue_family != vulkan_context.present_queue_family) {
			u32 queue_family_indices[] = { vulkan_context.graphics_queue_family, vulkan_context.present_queue_family };
			swapchain_create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
			swapchain_create_info.queueFamilyIndexCount = ARRAY_COUNT(queue_family_indices);
			swapchain_create_info.pQueueFamilyIndices   = queue_family_indices;
		} else {
			swapchain_create_info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
		}

		VK_CHECK(vkCreateSwapchainKHR(vulkan_context.device, &swapchain_create_info, NULL, &vulkan_context.swapchain));
	}

	// Swapchain images.
	{
		//u32 num_swapchain_images = 0;
		VK_CHECK(vkGetSwapchainImagesKHR(vulkan_context.device, vulkan_context.swapchain, &vulkan_context.num_swapchain_images, NULL));
		auto swapchain_images = allocate_array(arena, VkImage, vulkan_context.num_swapchain_images);
		VK_CHECK(vkGetSwapchainImagesKHR(vulkan_context.device, vulkan_context.swapchain, &vulkan_context.num_swapchain_images, swapchain_images));

		//debug_print("\tCreated swap image count: %u\n", num_swapchain_images);

		//Array<VkImageView> swapchain_image_views{num_created_swap_images};
		//swapchain_context->image_views.resize(num_swapchain_images);
		for (s32 i = 0; i < vulkan_context.num_swapchain_images; i++) {
			VkImageViewCreateInfo image_view_create_info = {};
			image_view_create_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			image_view_create_info.image                           = swapchain_images[i];
			image_view_create_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
			image_view_create_info.format                          = vulkan_context.surface_format.format;
			image_view_create_info.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
			image_view_create_info.subresourceRange.baseMipLevel   = 0;
			image_view_create_info.subresourceRange.levelCount     = 1;
			image_view_create_info.subresourceRange.baseArrayLayer = 0;
			image_view_create_info.subresourceRange.layerCount     = 1;
			VK_CHECK(vkCreateImageView(vulkan_context.device, &image_view_create_info, NULL, &vulkan_context.swapchain_image_views[i]));
		}
	}

	// Render pass.
	{
		VkAttachmentDescription attachment_description = {};
		attachment_description.format         = SHADOW_MAP_DEPTH_FORMAT;
		attachment_description.samples        = VK_SAMPLE_COUNT_1_BIT;
		attachment_description.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachment_description.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
		attachment_description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment_description.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment_description.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		VkAttachmentReference depth_attachment_reference = {};
		depth_attachment_reference.attachment = 0;
		depth_attachment_reference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass_description = {};
		subpass_description.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass_description.colorAttachmentCount    = 0;
		subpass_description.pDepthStencilAttachment = &depth_attachment_reference;

		VkSubpassDependency subpass_dependencies[2];

		subpass_dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
		subpass_dependencies[0].dstSubpass      = 0;
		subpass_dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		subpass_dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		subpass_dependencies[0].srcAccessMask   = VK_ACCESS_SHADER_READ_BIT;
		subpass_dependencies[0].dstAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		subpass_dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		subpass_dependencies[1].srcSubpass      = 0;
		subpass_dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
		subpass_dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		subpass_dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		subpass_dependencies[1].srcAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		subpass_dependencies[1].dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
		subpass_dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo render_pass_create_info = {};
		render_pass_create_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_create_info.attachmentCount = 1;
		render_pass_create_info.pAttachments    = &attachment_description;
		render_pass_create_info.subpassCount    = 1;
		render_pass_create_info.pSubpasses      = &subpass_description;
		render_pass_create_info.dependencyCount = ARRAY_COUNT(subpass_dependencies);
		render_pass_create_info.pDependencies   = subpass_dependencies;

		VK_CHECK(vkCreateRenderPass(vulkan_context.device, &render_pass_create_info, NULL, &vulkan_context.render_passes.shadow_map));
	}

	{
		VkAttachmentDescription color_attachment = {};
		color_attachment.format         = vulkan_context.surface_format.format;
		color_attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
		color_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription attachments[] = {
			color_attachment,
			depthAttachment,
		};

		VkAttachmentReference color_attachment_reference = {};
		color_attachment_reference.attachment = 0;
		color_attachment_reference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass_description = {};
		subpass_description.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass_description.colorAttachmentCount    = 1;
		subpass_description.pColorAttachments       = &color_attachment_reference;
		subpass_description.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency subpass_dependency = {};
		subpass_dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
		subpass_dependency.dstSubpass    = 0;
		subpass_dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpass_dependency.srcAccessMask = 0;
		subpass_dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo render_pass_create_info = {};
		render_pass_create_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_create_info.attachmentCount = ARRAY_COUNT(attachments);
		render_pass_create_info.pAttachments    = attachments;
		render_pass_create_info.subpassCount    = 1;
		render_pass_create_info.pSubpasses      = &subpass_description;
		render_pass_create_info.dependencyCount = 1;
		render_pass_create_info.pDependencies   = &subpass_dependency;

		VK_CHECK(vkCreateRenderPass(vulkan_context.device, &render_pass_create_info, NULL, &vulkan_context.render_pass));
	}

	// Pipelines.
	{
		// @TODO: Add depth bias to shadow map pipeline dynamic state.
		VkDynamicState dynamic_states[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
			VK_DYNAMIC_STATE_DEPTH_BIAS,
		};

		VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {};
		dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state_create_info.dynamicStateCount = ARRAY_COUNT(dynamic_states);
		dynamic_state_create_info.pDynamicStates = dynamic_states;

		VkVertexInputBindingDescription vertex_input_binding_description = {};
		vertex_input_binding_description.binding   = 0;
		vertex_input_binding_description.stride    = sizeof(Vertex);
		vertex_input_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription position_attribute_description;
		position_attribute_description.binding  = 0;
		position_attribute_description.location = 0;
		position_attribute_description.format   = VK_FORMAT_R32G32B32_SFLOAT;
		position_attribute_description.offset   = offsetof(Vertex, position);

		VkVertexInputAttributeDescription color_attribute_description;
		color_attribute_description.binding  = 0;
		color_attribute_description.location = 1;
		color_attribute_description.format   = VK_FORMAT_R32G32B32_SFLOAT;
		color_attribute_description.offset   = offsetof(Vertex, color);

		VkVertexInputAttributeDescription uv_attribute_description;
		uv_attribute_description.binding  = 0;
		uv_attribute_description.location = 2;
		uv_attribute_description.format   = VK_FORMAT_R32G32_SFLOAT;
		uv_attribute_description.offset   = offsetof(Vertex, uv);

		VkVertexInputAttributeDescription normal_attribute_description;
		normal_attribute_description.binding  = 0;
		normal_attribute_description.location = 3;
		normal_attribute_description.format   = VK_FORMAT_R32G32B32_SFLOAT;
		normal_attribute_description.offset   = offsetof(Vertex, normal);

		VkVertexInputAttributeDescription vertex_input_attribute_descriptions[] = {
			position_attribute_description,
			color_attribute_description,
			uv_attribute_description,
			normal_attribute_description,
		};

		// @TODO: Fix shadow map vertex input attributes (only needs position).
		VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {};
		vertex_input_state_create_info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_state_create_info.vertexBindingDescriptionCount   = 1;
		vertex_input_state_create_info.pVertexBindingDescriptions      = &vertex_input_binding_description;
		vertex_input_state_create_info.vertexAttributeDescriptionCount = ARRAY_COUNT(vertex_input_attribute_descriptions);
		vertex_input_state_create_info.pVertexAttributeDescriptions    = vertex_input_attribute_descriptions;

		VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info = {};
		input_assembly_create_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly_create_info.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = {};
		viewport.x        = 0.0f;
		viewport.y        = 0.0f;
		viewport.width    = (f32)vulkan_context.swapchain_image_extent.width;
		viewport.height   = (f32)vulkan_context.swapchain_image_extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = {0, 0};
		scissor.extent = vulkan_context.swapchain_image_extent;

		VkPipelineViewportStateCreateInfo viewport_state_create_info = {};
		viewport_state_create_info.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state_create_info.viewportCount = 1;
		viewport_state_create_info.pViewports    = &viewport;
		viewport_state_create_info.scissorCount  = 1;
		viewport_state_create_info.pScissors     = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = {};
		rasterization_state_create_info.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterization_state_create_info.depthClampEnable        = VK_FALSE;
		rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
		rasterization_state_create_info.polygonMode             = VK_POLYGON_MODE_FILL;
		rasterization_state_create_info.lineWidth               = 1.0f;
		rasterization_state_create_info.cullMode                = VK_CULL_MODE_BACK_BIT;
		rasterization_state_create_info.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterization_state_create_info.depthBiasEnable         = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisample_state_create_info = {};
		multisample_state_create_info.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_state_create_info.sampleShadingEnable   = VK_FALSE;
		multisample_state_create_info.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
		//multisample_state_create_info.minSampleShading      = 1.0f;
		//multisample_state_create_info.alphaToCoverageEnable = VK_FALSE;
		//multisample_state_create_info.alphaToOneEnable      = VK_FALSE;

		VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info = {};
		depth_stencil_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil_state_create_info.depthTestEnable = VK_TRUE;
		depth_stencil_state_create_info.depthWriteEnable = VK_TRUE;
		depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
		depth_stencil_state_create_info.depthBoundsTestEnable = VK_FALSE;
		depth_stencil_state_create_info.stencilTestEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
		color_blend_attachment_state.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		color_blend_attachment_state.blendEnable         = VK_FALSE;
		//color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		//color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		//color_blend_attachment_state.colorBlendOp        = VK_BLEND_OP_ADD;
		//color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		//color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		//color_blend_attachment_state.alphaBlendOp        = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {};
		color_blend_state_create_info.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blend_state_create_info.logicOpEnable     = VK_FALSE;
		color_blend_state_create_info.logicOp           = VK_LOGIC_OP_COPY;
		color_blend_state_create_info.attachmentCount   = 1;
		color_blend_state_create_info.pAttachments      = &color_blend_attachment_state;
		color_blend_state_create_info.blendConstants[0] = 0.0f;
		color_blend_state_create_info.blendConstants[1] = 0.0f;
		color_blend_state_create_info.blendConstants[2] = 0.0f;
		color_blend_state_create_info.blendConstants[3] = 0.0f;

		VkPushConstantRange push_constant_range = {};
		push_constant_range.offset = 0;
		push_constant_range.size = sizeof(s32);
		push_constant_range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		// Use pipeline layout to specify uniform layout.
		VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
		pipeline_layout_create_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.setLayoutCount         = 1;
		pipeline_layout_create_info.pSetLayouts            = &vulkan_context.descriptor_set_layout;
		pipeline_layout_create_info.pushConstantRangeCount = 1;
		pipeline_layout_create_info.pPushConstantRanges    = &push_constant_range;

		VK_CHECK(vkCreatePipelineLayout(vulkan_context.device, &pipeline_layout_create_info, NULL, &vulkan_context.pipeline_layout));
		VK_CHECK(vkCreatePipelineLayout(vulkan_context.device, &pipeline_layout_create_info, NULL, &vulkan_context.pipeline_layouts.shadow_map));

		Create_Graphics_Pipeline_Request requests[] = {
			{&vulkan_context.pipelines.textured_static, vulkan_context.pipeline_layout, vulkan_context.render_pass, &vulkan_context.shaders[TEXTURED_STATIC_SHADER]},
			{&vulkan_context.pipelines.shadow_map_static, vulkan_context.pipeline_layouts.shadow_map, vulkan_context.render_passes.shadow_map, &vulkan_context.shaders[SHADOW_MAP_STATIC_SHADER]},
		};

		u32 pipeline_count = ARRAY_COUNT(requests);

		// @TODO: Subarena.
		VkGraphicsPipelineCreateInfo *graphics_pipeline_create_infos = allocate_array(arena, VkGraphicsPipelineCreateInfo, pipeline_count);

		for (s32 i = 0; i < pipeline_count; i++) {
			VkPipelineShaderStageCreateInfo *shader_stage_create_infos = allocate_array(arena, VkPipelineShaderStageCreateInfo, requests[i].shaders->module_count);

			for (s32 j = 0; j < requests[i].shaders->module_count; j++) {
				shader_stage_create_infos[j].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				shader_stage_create_infos[j].stage  = requests[i].shaders->modules[j].stage;
				shader_stage_create_infos[j].module = requests[i].shaders->modules[j].module;
				shader_stage_create_infos[j].pName  = "main";
			}
		/*
			VkPipelineShaderStageCreateInfo vertex_shader_stage_create_info = {};
			vertex_shader_stage_create_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertex_shader_stage_create_info.stage  = VK_SHADER_STAGE_VERTEX_BIT;
			vertex_shader_stage_create_info.module = vertex_shader_module;
			vertex_shader_stage_create_info.pName  = "main";

			VkPipelineShaderStageCreateInfo fragment_shader_stage_create_info = {};
			fragment_shader_stage_create_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragment_shader_stage_create_info.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragment_shader_stage_create_info.module = fragment_shader_module;
			fragment_shader_stage_create_info.pName  = "main";

			VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_info[] = {vertex_shader_stage_create_info, fragment_shader_stage_create_info};
		*/
			//VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {};

			if (i == 0) {
				dynamic_state_create_info.dynamicStateCount = 2;
			}

			if (i == 1) {
				// No blend attachment states (no color attachments used)
				color_blend_state_create_info.attachmentCount = 0;
				// Cull front faces
				depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
				// Enable depth bias
				rasterization_state_create_info.depthBiasEnable = VK_TRUE;
				// Add depth bias to dynamic state, so we can change it at runtime
				dynamic_state_create_info.dynamicStateCount = 3;
			}
			graphics_pipeline_create_infos[i].sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			graphics_pipeline_create_infos[i].stageCount          = requests[i].shaders->module_count;
			graphics_pipeline_create_infos[i].pStages             = shader_stage_create_infos;
			graphics_pipeline_create_infos[i].pVertexInputState   = &vertex_input_state_create_info;
			graphics_pipeline_create_infos[i].pInputAssemblyState = &input_assembly_create_info;
			graphics_pipeline_create_infos[i].pViewportState      = &viewport_state_create_info;
			graphics_pipeline_create_infos[i].pRasterizationState = &rasterization_state_create_info;
			graphics_pipeline_create_infos[i].pMultisampleState   = &multisample_state_create_info;
			graphics_pipeline_create_infos[i].pColorBlendState    = &color_blend_state_create_info;
			graphics_pipeline_create_infos[i].pDepthStencilState  = &depth_stencil_state_create_info;
			graphics_pipeline_create_infos[i].pDynamicState       = &dynamic_state_create_info;
			graphics_pipeline_create_infos[i].layout              = requests[i].pipeline_layout;
			graphics_pipeline_create_infos[i].renderPass          = requests[i].render_pass;
			graphics_pipeline_create_infos[i].subpass             = 0;
			//graphics_pipeline_create_info.basePipelineIndex   = -1;
			//graphics_pipeline_create_info.basePipelineHandle  = VK_NULL_HANDLE;
			//graphics_pipeline_create_info.basePipelineIndex   = -1;
			if (i == 1) {
				graphics_pipeline_create_infos[i].stageCount = 1;
			}

			VK_CHECK(vkCreateGraphicsPipelines(vulkan_context.device, NULL, 1, &graphics_pipeline_create_infos[i], NULL, requests[i].pipeline));
		}

		//VkPipeline pipelines[pipeline_count];

		//for (s32 i = 0; i < pipeline_count; i++) {
			//*requests[i].pipeline = pipelines[i];
		//}
	}

	// Depth image.
	{
		// @TODO: Check to make sure the physical device supports the depth image format. Have fallback formats?

		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = vulkan_context.swapchain_image_extent.width;
		imageInfo.extent.height = vulkan_context.swapchain_image_extent.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		VK_CHECK(vkCreateImage(vulkan_context.device, &imageInfo, nullptr, &vulkan_context.depthImage));

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(vulkan_context.device, vulkan_context.depthImage, &memRequirements);

		allocate_vulkan_memory(&vulkan_context.depthImageMemory, memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		vkBindImageMemory(vulkan_context.device, vulkan_context.depthImage, vulkan_context.depthImageMemory, 0);

		VkImageViewCreateInfo image_view_create_info = {};
		image_view_create_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.image                           = vulkan_context.depthImage;
		image_view_create_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.format                          = VK_FORMAT_D32_SFLOAT_S8_UINT;
		image_view_create_info.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
		image_view_create_info.subresourceRange.baseMipLevel   = 0;
		image_view_create_info.subresourceRange.levelCount     = 1;
		image_view_create_info.subresourceRange.baseArrayLayer = 0;
		image_view_create_info.subresourceRange.layerCount     = 1;
		VK_CHECK(vkCreateImageView(vulkan_context.device, &image_view_create_info, NULL, &vulkan_context.depthImageView));

		transitionImageLayout(vulkan_context.depthImage, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}

	// Framebuffers.
	{
		for (s32 i = 0; i < vulkan_context.num_swapchain_images; ++i) {
			VkImageView attachments[] = {
				vulkan_context.swapchain_image_views[i],
				vulkan_context.depthImageView,
			};

			VkFramebufferCreateInfo framebuffer_create_info = {};
			framebuffer_create_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebuffer_create_info.renderPass      = vulkan_context.render_pass;
			framebuffer_create_info.attachmentCount = ARRAY_COUNT(attachments);
			framebuffer_create_info.pAttachments    = attachments;
			framebuffer_create_info.width           = vulkan_context.swapchain_image_extent.width;
			framebuffer_create_info.height          = vulkan_context.swapchain_image_extent.height;
			framebuffer_create_info.layers          = 1;

			VK_CHECK(vkCreateFramebuffer(vulkan_context.device, &framebuffer_create_info, NULL, &vulkan_context.framebuffers[i]));
		}
	}

	{
		VkFormat fbColorFormat = SHADOW_MAP_COLOR_FORMAT;

		// For shadow mapping we only need a depth attachment
		VkImageCreateInfo image = {};
		image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image.imageType = VK_IMAGE_TYPE_2D;
		image.extent.width = SHADOW_MAP_WIDTH;
		image.extent.height = SHADOW_MAP_HEIGHT;
		image.extent.depth = 1;
		image.mipLevels = 1;
		image.arrayLayers = 1;
		image.samples = VK_SAMPLE_COUNT_1_BIT;
		image.tiling = VK_IMAGE_TILING_OPTIMAL;
		image.format = SHADOW_MAP_DEPTH_FORMAT;																// Depth stencil attachment
		image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;		// We will sample directly from the depth attachment for the shadow mapping
		VK_CHECK(vkCreateImage(vulkan_context.device, &image, NULL, &vulkan_context.framebuffer_attachments.shadow_map_depth.image));

		VkMemoryRequirements memReqs;
		vkGetImageMemoryRequirements(vulkan_context.device, vulkan_context.framebuffer_attachments.shadow_map_depth.image, &memReqs);
		allocate_vulkan_memory(&vulkan_context.framebuffer_attachments.shadow_map_depth.memory, memReqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VK_CHECK(vkBindImageMemory(vulkan_context.device, vulkan_context.framebuffer_attachments.shadow_map_depth.image, vulkan_context.framebuffer_attachments.shadow_map_depth.memory, 0));

		VkImageViewCreateInfo depthStencilView = {};
		depthStencilView.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
		depthStencilView.format = SHADOW_MAP_DEPTH_FORMAT;
		depthStencilView.subresourceRange = {};
		depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		depthStencilView.subresourceRange.baseMipLevel = 0;
		depthStencilView.subresourceRange.levelCount = 1;
		depthStencilView.subresourceRange.baseArrayLayer = 0;
		depthStencilView.subresourceRange.layerCount = 1;
		depthStencilView.image = vulkan_context.framebuffer_attachments.shadow_map_depth.image;
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

u32 ubo_offset, sm_ubo_offset, dubo_offset;

void build_vulkan_command_buffers(Game_Assets *assets) {
	VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
	command_buffer_allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_info.commandPool        = vulkan_context.command_pool;
	command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_allocate_info.commandBufferCount = vulkan_context.num_swapchain_images;
	VK_CHECK(vkAllocateCommandBuffers(vulkan_context.device, &command_buffer_allocate_info, vulkan_context.command_buffers));

	VkClearValue clear_color = {0.5f, 0.5f, 0.5f, 1.0f};
	VkClearValue clear_depth_stencil = {1.0f, 0.0f};
	VkClearValue clear_values[] = {
		clear_color,
		clear_depth_stencil,
	};

	VkBuffer vertex_buffers[] = {vulkan_context.gpu_buffer};
	VkDeviceSize offsets[] = {0};

	// Fill command buffers.
	for (u32 i = 0; i < vulkan_context.num_swapchain_images; i++) {
		VkCommandBufferBeginInfo command_buffer_begin_info = {};
		command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		VK_CHECK(vkBeginCommandBuffer(vulkan_context.command_buffers[i], &command_buffer_begin_info));

		// Shadow map.
		{
			VkRenderPassBeginInfo render_pass_begin_info = {};
			render_pass_begin_info.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			render_pass_begin_info.renderPass = vulkan_context.render_passes.shadow_map;
			render_pass_begin_info.framebuffer = vulkan_context.xframebuffers.shadow_map;
			render_pass_begin_info.renderArea.extent.width = SHADOW_MAP_WIDTH;
			render_pass_begin_info.renderArea.extent.height = SHADOW_MAP_HEIGHT;
			render_pass_begin_info.clearValueCount = 1;
			render_pass_begin_info.pClearValues = &clear_depth_stencil;

			vkCmdBeginRenderPass(vulkan_context.command_buffers[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport = {};
			viewport.x        = 0.0f;
			viewport.y        = 0.0f;
			viewport.width    = (f32)SHADOW_MAP_WIDTH;
			viewport.height   = (f32)SHADOW_MAP_HEIGHT;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(vulkan_context.command_buffers[i], 0, 1, &viewport);

			VkRect2D scissor = {};
			scissor.offset = {0, 0};
			scissor.extent = {SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT};
			vkCmdSetScissor(vulkan_context.command_buffers[i], 0, 1, &scissor);

			// Set depth bias, AKA polygon offset. Required to avoid shadow mapping artifacts.
			vkCmdSetDepthBias(vulkan_context.command_buffers[i], CONSTANT_DEPTH_BIAS, 0.0f, SLOPE_DEPTH_BIAS);

			vkCmdBindPipeline(vulkan_context.command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan_context.pipelines.shadow_map_static);

			u32 dynamic_offset = 0;
			vkCmdBindDescriptorSets(vulkan_context.command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan_context.pipeline_layouts.shadow_map, 0, 1, &vulkan_context.xdescriptor_sets.shadow_map, 1, &dynamic_offset);

			vkCmdBindVertexBuffers(vulkan_context.command_buffers[i], 0, 1, vertex_buffers, offsets);
			vkCmdBindIndexBuffer(vulkan_context.command_buffers[i], vulkan_context.gpu_buffer, ((Model_Asset *)assets->lookup[0])->meshes[0].vertex_count * sizeof(Vertex), VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(vulkan_context.command_buffers[i], ((Model_Asset *)assets->lookup[0])->meshes[0].index_count, 1, 0, 0, 0);

			vkCmdEndRenderPass(vulkan_context.command_buffers[i]);
		}
//////////////////////////////////////////

		// Scene.
		{
			VkRenderPassBeginInfo render_pass_begin_info = {};
			render_pass_begin_info.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			render_pass_begin_info.renderPass        = vulkan_context.render_pass;
			render_pass_begin_info.framebuffer       = vulkan_context.framebuffers[i];
			render_pass_begin_info.renderArea.offset = {0, 0};
			render_pass_begin_info.renderArea.extent = vulkan_context.swapchain_image_extent;
			render_pass_begin_info.clearValueCount   = ARRAY_COUNT(clear_values);
			render_pass_begin_info.pClearValues      = clear_values;

			vkCmdBeginRenderPass(vulkan_context.command_buffers[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport = {};
			viewport.x        = 0.0f;
			viewport.y        = 0.0f;
			viewport.width    = (f32)vulkan_context.swapchain_image_extent.width;
			viewport.height   = (f32)vulkan_context.swapchain_image_extent.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(vulkan_context.command_buffers[i], 0, 1, &viewport);

			VkRect2D scissor = {};
			scissor.offset = {0, 0};
			scissor.extent = {vulkan_context.swapchain_image_extent.width, vulkan_context.swapchain_image_extent.height};
			vkCmdSetScissor(vulkan_context.command_buffers[i], 0, 1, &scissor);

			vkCmdBindPipeline(vulkan_context.command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan_context.pipelines.textured_static);

			s32 push_constant = 0;
			vkCmdPushConstants(vulkan_context.command_buffers[i], vulkan_context.pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(s32), (void *)&push_constant);

			// @TODO: Use a single buffer with offsets.
			vkCmdBindVertexBuffers(vulkan_context.command_buffers[i], 0, 1, vertex_buffers, offsets);
			vkCmdBindIndexBuffer(vulkan_context.command_buffers[i], vulkan_context.gpu_buffer, ((Model_Asset *)assets->lookup[0])->meshes[0].vertex_count * sizeof(Vertex), VK_INDEX_TYPE_UINT32);
			u32 dynamic_offset = 0;
			vkCmdBindDescriptorSets(vulkan_context.command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan_context.pipeline_layout, 0, 1, &vulkan_context.descriptor_sets[i], 1, &dynamic_offset);

			//vkCmdDraw(vulkan_context.command_buffers[i], ARRAY_COUNT(vertices), 1, 0, 0);
			vkCmdDrawIndexed(vulkan_context.command_buffers[i], ((Model_Asset *)assets->lookup[0])->meshes[0].index_count, 1, 0, 0, 0);

			vkCmdEndRenderPass(vulkan_context.command_buffers[i]);
		}

		VK_CHECK(vkEndCommandBuffer(vulkan_context.command_buffers[i]));
	}
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

void copy_vulkan_buffer(VkBuffer source, VkBuffer destination, VkDeviceSize size) {
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
	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = 0; // Optional
	copyRegion.size = size;
	vkCmdCopyBuffer(command_buffer, source, destination, 1, &copyRegion);
	vkEndCommandBuffer(command_buffer);
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &command_buffer;

	vkQueueSubmit(vulkan_context.graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(vulkan_context.graphics_queue); // @TODO: Use a fence?
	vkFreeCommandBuffers(vulkan_context.device, vulkan_context.command_pool, 1, &command_buffer);
}

struct UBO {
};

#define TEST_INSTANCES 10

struct Dynamic_Scene_UBO {
	M4 world_to_clip_space;
	M4 world_to_shadow_map_clip_space;
} dynamic_scene_ubo[TEST_INSTANCES];

struct {
	alignas(16) M4 world_to_clip_space;
} shadow_map_ubo;

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferCopy copyRegion = {};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(commandBuffer);
    }

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        endSingleTimeCommands(commandBuffer);
    }

void load_model(Model_Asset *model, const char *path);

void load_vulkan_texture(u8 *pixels, s32 texture_width, s32 texture_height) {
	VkDeviceSize imageSize = texture_width * texture_height * 4;
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	create_vulkan_buffer(&stagingBuffer, &stagingBufferMemory, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	void* data;
	vkMapMemory(vulkan_context.device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(vulkan_context.device, stagingBufferMemory);

	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = static_cast<uint32_t>(texture_width);
	imageInfo.extent.height = static_cast<uint32_t>(texture_height);
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	VK_CHECK(vkCreateImage(vulkan_context.device, &imageInfo, NULL, &vulkan_context.textureImage));

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(vulkan_context.device, vulkan_context.textureImage, &memRequirements);

	allocate_vulkan_memory(&vulkan_context.textureImageMemory, memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vkBindImageMemory(vulkan_context.device, vulkan_context.textureImage, vulkan_context.textureImageMemory, 0);

	transitionImageLayout(vulkan_context.textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copyBufferToImage(stagingBuffer, vulkan_context.textureImage, static_cast<uint32_t>(texture_width), static_cast<uint32_t>(texture_height));
	transitionImageLayout(vulkan_context.textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(vulkan_context.device, stagingBuffer, NULL);
	vkFreeMemory(vulkan_context.device, stagingBufferMemory, NULL);

	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = vulkan_context.textureImage;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;
	VK_CHECK(vkCreateImageView(vulkan_context.device, &viewInfo, NULL, &vulkan_context.textureImageView));
}

u32 align_to(u32 size, u32 alignment) {
	u32 remainder = size % alignment;
	if (remainder == 0) {
		return size;
	}
	return size + alignment - remainder;
}

void transfer_model_data_to_gpu(Model_Asset *model) {
	auto total_data_size = 0;

	for (s32 i = 0; i < model->mesh_count; i++) {
		auto vertex_size = model->meshes[i].vertex_count * sizeof(Vertex);
		auto index_size = model->meshes[i].index_count * sizeof(u32);
		auto data_size = vertex_size + index_size;

		void *data;
		VK_CHECK(vkMapMemory(vulkan_context.device, vulkan_context.shared_memory, vulkan_context.vertex_index_offset, data_size, 0, &data));
		memcpy(data, model->meshes[i].vertices, vertex_size);
		memcpy(((char *)data) + vertex_size, model->meshes[i].indices, index_size);
		vkUnmapMemory(vulkan_context.device, vulkan_context.shared_memory);

		vulkan_context.vertex_index_offset += data_size;
		total_data_size += data_size;
	}

	// @TODO: Batch copies?
	copy_vulkan_buffer(vulkan_context.staging_buffer, vulkan_context.gpu_buffer, total_data_size);
}

void do_ds() {
	// Descriptor set.
	{
		VkDescriptorSetLayout layouts[] = {
			vulkan_context.descriptor_set_layout,
			vulkan_context.descriptor_set_layout,
			vulkan_context.descriptor_set_layout,
		};
		VkDescriptorSetLayout layouts2[] = {
			vulkan_context.descriptor_set_layout,
		};

		VkDescriptorSetAllocateInfo allocate_info = {};
		allocate_info.sType               = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocate_info.descriptorPool      = vulkan_context.descriptor_pool;
		allocate_info.descriptorSetCount  = vulkan_context.num_swapchain_images;
		allocate_info.pSetLayouts         = layouts;
		VK_CHECK(vkAllocateDescriptorSets(vulkan_context.device, &allocate_info, vulkan_context.descriptor_sets));

		VkDescriptorSetAllocateInfo allocate_info2 = {};
		allocate_info2.sType               = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocate_info2.descriptorPool      = vulkan_context.descriptor_pool;
		allocate_info2.descriptorSetCount  = 1;
		allocate_info2.pSetLayouts         = layouts2;
		VK_CHECK(vkAllocateDescriptorSets(vulkan_context.device, &allocate_info2, &vulkan_context.xdescriptor_sets.shadow_map));

		{
			VkDescriptorBufferInfo buffer_info = {};
			buffer_info.buffer  = vulkan_context.gpu_buffer;
			buffer_info.offset  = sm_ubo_offset;
			buffer_info.range   = sizeof(shadow_map_ubo);

			VkWriteDescriptorSet uboDescriptorWrite = {};
			uboDescriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			uboDescriptorWrite.dstSet          = vulkan_context.xdescriptor_sets.shadow_map;
			uboDescriptorWrite.dstBinding      = 0;
			uboDescriptorWrite.dstArrayElement = 0;
			uboDescriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uboDescriptorWrite.descriptorCount = 1;
			uboDescriptorWrite.pBufferInfo     = &buffer_info;

			vkUpdateDescriptorSets(vulkan_context.device, 1, &uboDescriptorWrite, 0, NULL);
		}

		for (s32 i = 0; i < vulkan_context.num_swapchain_images; i++) {
			VkDescriptorBufferInfo buffer_info = {};
			buffer_info.buffer  = vulkan_context.gpu_buffer;
			buffer_info.offset  = align_to(ubo_offset + (sizeof(UBO) * i), vulkan_context.minimum_uniform_buffer_offset_alignment);
			//buffer_info.offset  = ubo_offset;
			buffer_info.range   = sizeof(UBO);

			VkDescriptorBufferInfo dynamic_buffer_info = {};
			dynamic_buffer_info.buffer  = vulkan_context.gpu_buffer;
			dynamic_buffer_info.offset  = dubo_offset;
			//printf("buffer info offset: %lu\n", buffer_info.offset);
			//buffer_info.offset  = ubo_offset;
			dynamic_buffer_info.range   = sizeof(dynamic_scene_ubo);

			for (s32 j = 0; j < VULKAN_MAX_TEXTURE_COUNT; j++) {
				//VkDescriptorImageInfo image_info = {};
				vulkan_context.image_infos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				vulkan_context.image_infos[j].imageView = vulkan_context.textureImageView;
				vulkan_context.image_infos[j].sampler = NULL;
			}

			VkWriteDescriptorSet uboDescriptorWrite = {};
			uboDescriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			uboDescriptorWrite.dstSet          = vulkan_context.descriptor_sets[i];
			uboDescriptorWrite.dstBinding      = 0;
			uboDescriptorWrite.dstArrayElement = 0;
			uboDescriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uboDescriptorWrite.descriptorCount = 1;
			uboDescriptorWrite.pBufferInfo     = &buffer_info;

			VkWriteDescriptorSet dynamic_ubo_descriptor_write = {};
			dynamic_ubo_descriptor_write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			dynamic_ubo_descriptor_write.dstSet          = vulkan_context.descriptor_sets[i];
			dynamic_ubo_descriptor_write.dstBinding      = 1;
			dynamic_ubo_descriptor_write.dstArrayElement = 0;
			dynamic_ubo_descriptor_write.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			dynamic_ubo_descriptor_write.descriptorCount = 1;
			dynamic_ubo_descriptor_write.pBufferInfo     = &dynamic_buffer_info;

			VkDescriptorImageInfo sampler_info = {};
			sampler_info.sampler = vulkan_context.textureSampler;

			VkWriteDescriptorSet samplerDescriptorWrite = {};
			samplerDescriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			samplerDescriptorWrite.dstSet          = vulkan_context.descriptor_sets[i];
			samplerDescriptorWrite.dstBinding      = 2;
			samplerDescriptorWrite.dstArrayElement = 0;
			samplerDescriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER;
			samplerDescriptorWrite.descriptorCount = 1;
			samplerDescriptorWrite.pImageInfo      = &sampler_info;

			VkDescriptorImageInfo smimage_info = {};
			smimage_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			smimage_info.imageView = vulkan_context.framebuffer_attachments.shadow_map_depth.image_view;
			smimage_info.sampler = vulkan_context.samplers.shadow_map_depth;

			VkWriteDescriptorSet smsamplerDescriptorWrite = {};
			smsamplerDescriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			smsamplerDescriptorWrite.dstSet          = vulkan_context.descriptor_sets[i];
			smsamplerDescriptorWrite.dstBinding      = 3;
			smsamplerDescriptorWrite.dstArrayElement = 0;
			smsamplerDescriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			smsamplerDescriptorWrite.descriptorCount = 1;
			smsamplerDescriptorWrite.pImageInfo      = &smimage_info;

			VkWriteDescriptorSet textures_descriptor_write = {};
			textures_descriptor_write = {};
			textures_descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			textures_descriptor_write.dstBinding = 4;
			textures_descriptor_write.dstArrayElement = 0;
			textures_descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			textures_descriptor_write.descriptorCount = VULKAN_MAX_TEXTURE_COUNT;
			textures_descriptor_write.pBufferInfo = 0;
			textures_descriptor_write.dstSet = vulkan_context.descriptor_sets[i];
			textures_descriptor_write.pImageInfo = vulkan_context.image_infos;

			VkWriteDescriptorSet descriptor_writes[] = {
				uboDescriptorWrite,
				dynamic_ubo_descriptor_write,
				samplerDescriptorWrite,
				smsamplerDescriptorWrite,
				textures_descriptor_write,
			};

			vkUpdateDescriptorSets(vulkan_context.device, ARRAY_COUNT(descriptor_writes), descriptor_writes, 0, NULL);
		}
	}
}

struct Create_Shader_Request {
	Shaders *shaders;
	const char **paths;
	u32 shader_module_count;
};

// @TODO: Use a temporary arena.
void initialize_renderer(Game_State *game_state) {
	//load_model(NULL, "data/male2.fbx");

	//Library_Handle vulkan_library = open_shared_library("dependencies/vulkan/1.1.106.0/lib/libvulkan.so");
	if (SDL_Vulkan_LoadLibrary("dependencies/vulkan/1.1.106.0/lib/libvulkan.so")) {
		_abort("SDL could not load Vulkan library: %s", SDL_GetError());
	}

	s32 num_required_device_extensions = 0;
	const char *required_device_extensions[10]; // @TEMP
	required_device_extensions[num_required_device_extensions++] = "VK_KHR_swapchain";

	s32 num_required_instance_layers = 0;
	const char *required_instance_layers[10]; // @TEMP

	s32 num_required_instance_extensions = 0;
	const char *required_instance_extensions[10]; // @TEMP
	required_instance_extensions[num_required_instance_extensions++] = "VK_KHR_surface";
	required_instance_extensions[num_required_instance_extensions++] = "VK_KHR_xlib_surface";

	if (debug) {
		required_instance_extensions[num_required_instance_extensions++] = "VK_EXT_debug_utils";
		required_instance_layers[num_required_instance_layers++] = "VK_LAYER_KHRONOS_validation";
	}

#define VK_EXPORTED_FUNCTION(name)\
	name = (PFN_##name)SDL_Vulkan_GetVkGetInstanceProcAddr();\
	if (!name) _abort("Failed to load Vulkan function %s: Vulkan version 1.1 required", #name);
#define VK_GLOBAL_FUNCTION(name)\
	name = (PFN_##name)vkGetInstanceProcAddr(NULL, (const char *)#name);\
	if (!name) _abort("Failed to load Vulkan function %s: Vulkan version 1.1 required", #name);
#define VK_INSTANCE_FUNCTION(name)
#define VK_DEVICE_FUNCTION(name)

#include "vulkan_functions.h"

#undef VK_EXPORTED_FUNCTION
#undef VK_GLOBAL_FUNCTION
#undef VK_INSTANCE_FUNCTION
#undef VK_DEVICE_FUNCTION

	u32 version;
	vkEnumerateInstanceVersion(&version);
	if (VK_VERSION_MAJOR(version) == 1 && VK_VERSION_MINOR(version) < 1) {
		_abort("Vulkan version 1.1 or greater required: version %d.%d.%d is installed");
	}
	debug_print("Using Vulkan version %d.%d.%d\n", VK_VERSION_MAJOR(version), VK_VERSION_MINOR(version), VK_VERSION_PATCH(version));

	VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {};
	if (debug) {
		debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debug_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debug_create_info.pfnUserCallback = vulkan_debug_message_callback;
	}

	// Create instance.
	{
		u32 num_available_instance_layers;
		vkEnumerateInstanceLayerProperties(&num_available_instance_layers, NULL);
		VkLayerProperties *available_instance_layers = (VkLayerProperties *)malloc(sizeof(VkLayerProperties) * num_available_instance_layers); // @TEMP
		vkEnumerateInstanceLayerProperties(&num_available_instance_layers, available_instance_layers);

		debug_print("Available Vulkan layers:\n");
		for (s32 i = 0; i < num_available_instance_layers; i++) {
			debug_print("\t%s\n", available_instance_layers[i].layerName);
		}

		u32 num_available_instance_extensions = 0;
		VK_CHECK(vkEnumerateInstanceExtensionProperties(NULL, &num_available_instance_extensions, NULL));
		VkExtensionProperties *available_instance_extensions = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * num_available_instance_extensions); // @TEMP
		VK_CHECK(vkEnumerateInstanceExtensionProperties(NULL, &num_available_instance_extensions, available_instance_extensions));

		debug_print("Available Vulkan extensions:\n");
		for (s32 i = 0; i < num_available_instance_extensions; i++) {
			debug_print("\t%s\n", available_instance_extensions[i].extensionName);
		}

		VkApplicationInfo application_info = {};
		application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		application_info.pApplicationName = "Jaguar";
		application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		application_info.pEngineName = "Jaguar";
		application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		application_info.apiVersion = VK_MAKE_VERSION(1, 1, 0);

		// @TODO: Check that all required extensions/layers are available.
		VkInstanceCreateInfo instance_create_info = {};
		instance_create_info.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_create_info.pApplicationInfo        = &application_info;
		instance_create_info.enabledLayerCount       = num_required_instance_layers;
		instance_create_info.ppEnabledLayerNames     = required_instance_layers;
		instance_create_info.enabledExtensionCount   = num_required_instance_extensions;
		instance_create_info.ppEnabledExtensionNames = required_instance_extensions;
		if (debug) {
			instance_create_info.pNext = &debug_create_info;
		}
		VK_CHECK(vkCreateInstance(&instance_create_info, NULL, &vulkan_context.instance));
	}

#define VK_EXPORTED_FUNCTION(name)
#define VK_GLOBAL_FUNCTION(name)
#define VK_INSTANCE_FUNCTION(name)\
	name = (PFN_##name)vkGetInstanceProcAddr(vulkan_context.instance, (const char *)#name);\
	if (!name) _abort("Failed to load Vulkan function %s: Vulkan version 1.1 required", #name);
#define VK_DEVICE_FUNCTION(name)

#include "vulkan_functions.h"

#undef VK_EXPORTED_FUNCTION
#undef VK_GLOBAL_FUNCTION
#undef VK_INSTANCE_FUNCTION
#undef VK_DEVICE_FUNCTION

	if (debug) {
		VK_CHECK(vkCreateDebugUtilsMessengerEXT(vulkan_context.instance, &debug_create_info, NULL, &vulkan_context.debug_messenger));
	}

	// Create surface.
	{
		// @TODO: Make this platform generic.
		// Type-def the surface creation types and create wrapper calls in the platform layer.
		VkXlibSurfaceCreateInfoKHR surface_create_info = {};
		surface_create_info.sType  = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
		surface_create_info.dpy    = linux_context.display;
		surface_create_info.window = linux_context.window;
		VK_CHECK(vkCreateXlibSurfaceKHR(vulkan_context.instance, &surface_create_info, NULL, &vulkan_context.surface));
	}

	// Select physical device.
	// @TODO: Rank physical device and select the best one?
	{
		u8 found_suitable_physical_device = false;

		u32 num_physical_devices = 0;
		VK_CHECK(vkEnumeratePhysicalDevices(vulkan_context.instance, &num_physical_devices, NULL));
		if (num_physical_devices == 0) {
			_abort("Could not find any physical devices.");
		}
		//debug_print("Found %u physical devices.\n", num_physical_devices);
		VkPhysicalDevice physical_devices[10];// = allocate_array(&temporary_memory_arena, VkPhysicalDevice, num_physical_devices);
		VK_CHECK(vkEnumeratePhysicalDevices(vulkan_context.instance, &num_physical_devices, physical_devices));

		for (s32 i = 0; i < num_physical_devices; i++) {
			VkPhysicalDeviceProperties physical_device_properties;
			vkGetPhysicalDeviceProperties(physical_devices[i], &physical_device_properties);
			//debug_print("\tDevice type = %s\n", vk_physical_device_type_to_string(physical_device_properties.deviceType));
			VkPhysicalDeviceFeatures physical_device_features;
			vkGetPhysicalDeviceFeatures(physical_devices[i], &physical_device_features);
			//debug_print("\tHas geometry shader = %d\n", physical_device_features.geometryShader);
			//if (physical_device_properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || !physical_device_features.geometryShader) {
			if (!physical_device_features.samplerAnisotropy) {
				//debug_print("Skipping physical device because it is not a discrete gpu.\n");
				continue;
			}

			u32 num_available_device_extensions = 0;
			VK_CHECK(vkEnumerateDeviceExtensionProperties(physical_devices[i], NULL, &num_available_device_extensions, NULL));
			//debug_print("\tNumber of available device extensions: %u\n", num_available_device_extensions);
			auto available_device_extensions = allocate_array(&game_state->frame_arena, VkExtensionProperties, num_available_device_extensions);
			VK_CHECK(vkEnumerateDeviceExtensionProperties(physical_devices[i], NULL, &num_available_device_extensions, available_device_extensions));

			//if (!strings_subset_test(required_device_extensions, num_required_device_extensions, available_device_extensions, num_available_device_extensions)) {
				//continue;
			//}
			u8 missing_required_device_extension = false;
			for (s32 j = 0; j < num_required_device_extensions; j++) {
				u8 found = false;
				for (s32 k = 0; k < num_available_device_extensions; k++) {
					//debug_print("\t\t%s\n", available_ext.extensionName);
					if (compare_strings(available_device_extensions[k].extensionName, required_device_extensions[j]) == 0) {
						found = true;
						break;
					}
				}
				if (!found) {
					//debug_print("\tCould not find required device extension '%s'.\n", required_ext);
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
			u32 num_available_surface_formats = 0;
			VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_devices[i], vulkan_context.surface, &num_available_surface_formats, NULL));
			u32 num_available_present_modes = 0;
			VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_devices[i], vulkan_context.surface, &num_available_present_modes, NULL));
			if (num_available_surface_formats == 0 || num_available_present_modes == 0) {
				//debug_print("\tSkipping physical device because no supported format or present mode for this surface.\n\t\num_available_surface_formats = %u\n\t\num_available_present_modes = %u\n", num_available_surface_formats, num_available_present_modes);
				continue;
			}
			//debug_print("\tPhysical device is compatible with surface.\n");

			// Select the best swap chain settings.
			auto available_surface_formats = allocate_array(&game_state->frame_arena, VkSurfaceFormatKHR, num_available_surface_formats);
			VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_devices[i], vulkan_context.surface, &num_available_surface_formats, available_surface_formats));
			auto surface_format = available_surface_formats[0];
			//debug_print("\tAvailable surface formats (VkFormat, VkColorSpaceKHR):\n");
			if (num_available_surface_formats == 1 && available_surface_formats[0].format == VK_FORMAT_UNDEFINED) {
				// No preferred format, so we get to pick our own.
				surface_format = {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
				//debug_print("\t\tNo preferred format.\n");
			} else {
				for (s32 j = 0; j < num_available_surface_formats; j++) {
					if (available_surface_formats[j].format == VK_FORMAT_B8G8R8A8_UNORM && available_surface_formats[j].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
						surface_format = available_surface_formats[j];
						break;
					}
					//printf("\t\t%d, %d\n", asf.format, asf.colorSpace);
				}
			}

			VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
			auto available_present_modes = allocate_array(&game_state->frame_arena, VkPresentModeKHR, num_available_present_modes);
			VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_devices[i], vulkan_context.surface, &num_available_present_modes, available_present_modes));
			//debug_print("\tAvailable present modes:\n");
			for (s32 j = 0; j < num_available_present_modes; j++) {
				if (available_present_modes[j] == VK_PRESENT_MODE_MAILBOX_KHR) {
					present_mode = available_present_modes[j];
					break;
				}
				//debug_print("\t\t%d\n", apm);
			}

			u32 num_queue_families = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[i], &num_queue_families, NULL);
			auto queue_families = allocate_array(&game_state->frame_arena, VkQueueFamilyProperties, num_queue_families);
			vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[i], &num_queue_families, queue_families);

			//debug_print("\tFound %u queue families.\n", num_queue_families);

			// @TODO: Search for an transfer exclusive queue VK_QUEUE_TRANSFER_BIT.
			u32 graphics_queue_family = UINT32_MAX;
			u32 present_queue_family = UINT32_MAX;
			for (u32 j = 0; j < num_queue_families; j++) {
				if (queue_families[j].queueCount > 0) {
					if (queue_families[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
						graphics_queue_family = j;
					u32 present_support = false;
					VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(physical_devices[i], j, vulkan_context.surface, &present_support));
					if (present_support) {
						present_queue_family = j;
					}
					if (graphics_queue_family != UINT32_MAX && present_queue_family != UINT32_MAX) {
						vulkan_context.physical_device = physical_devices[i];
						vulkan_context.graphics_queue_family = graphics_queue_family;
						vulkan_context.present_queue_family = present_queue_family;
						vulkan_context.surface_format = surface_format;
						vulkan_context.present_mode = present_mode;

						//assert(physical_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && physical_device_features.geometryShader);
						//debug_print("\nSelected physical device %d.\n", query_count);
						//debug_print("\tVkFormat: %d\n\tVkColorSpaceKHR %d\n", surface_format.format, surface_format.colorSpace);
						//debug_print("\tVkPresentModeKHR %d\n", present_mode);

						found_suitable_physical_device = true;
						vulkan_context.minimum_uniform_buffer_offset_alignment = physical_device_properties.limits.minUniformBufferOffsetAlignment;
						break;
					}
				}
			}
			if (found_suitable_physical_device) {
				break;
			}
		}
		if (!found_suitable_physical_device) {
			_abort("Could not find suitable physical device.\n");
		}
	}

	// Create logical device.
	{
		u32 num_unique_queue_families = 1;
		if (vulkan_context.graphics_queue_family != vulkan_context.present_queue_family) {
			num_unique_queue_families = 2;
		}

		f32 queue_priority = 1.0f;

		auto device_queue_create_info = allocate_array(&game_state->frame_arena, VkDeviceQueueCreateInfo, num_unique_queue_families);
		device_queue_create_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		device_queue_create_info[0].queueFamilyIndex = vulkan_context.graphics_queue_family;
		device_queue_create_info[0].queueCount = 1;
		device_queue_create_info[0].pQueuePriorities = &queue_priority;

		if (vulkan_context.present_queue_family != vulkan_context.graphics_queue_family) {
			device_queue_create_info[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			device_queue_create_info[1].queueFamilyIndex = vulkan_context.present_queue_family;
			device_queue_create_info[1].queueCount = 1;
			device_queue_create_info[1].pQueuePriorities = &queue_priority;
		}

		VkPhysicalDeviceFeatures physical_device_features = {};
		physical_device_features.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo device_create_info = {};
		device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_create_info.pQueueCreateInfos = device_queue_create_info;
		device_create_info.queueCreateInfoCount = num_unique_queue_families;
		device_create_info.pEnabledFeatures = &physical_device_features;
		device_create_info.enabledExtensionCount = num_required_device_extensions;
		device_create_info.ppEnabledExtensionNames = required_device_extensions;
		device_create_info.enabledLayerCount = num_required_instance_layers;
		device_create_info.ppEnabledLayerNames = required_instance_layers;

		VK_CHECK(vkCreateDevice(vulkan_context.physical_device, &device_create_info, NULL, &vulkan_context.device));
	}

#define VK_EXPORTED_FUNCTION(name)
#define VK_GLOBAL_FUNCTION(name)
#define VK_INSTANCE_FUNCTION(name)
#define VK_DEVICE_FUNCTION(name)\
	name = (PFN_##name)vkGetDeviceProcAddr(vulkan_context.device, (const char *)#name);\
	if (!name) _abort("Failed to load Vulkan function %s: Vulkan version 1.1 required", #name);

#include "vulkan_functions.h"

#undef VK_EXPORTED_FUNCTION
#undef VK_GLOBAL_FUNCTION
#undef VK_INSTANCE_FUNCTION
#undef VK_DEVICE_FUNCTION

	// Create queues.
	{
		vkGetDeviceQueue(vulkan_context.device, vulkan_context.graphics_queue_family, 0, &vulkan_context.graphics_queue);
		vkGetDeviceQueue(vulkan_context.device, vulkan_context.present_queue_family, 0, &vulkan_context.present_queue);
	}

	// Descriptor set layout.
	{
		VkDescriptorSetLayoutBinding ubo_layout_binding = {};
		ubo_layout_binding.binding            = 0;
		ubo_layout_binding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		ubo_layout_binding.descriptorCount    = 1;
		ubo_layout_binding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
		ubo_layout_binding.pImmutableSamplers = NULL;

		VkDescriptorSetLayoutBinding dynamic_ubo_layout_binding = {};
		dynamic_ubo_layout_binding.binding            = 1;
		dynamic_ubo_layout_binding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		dynamic_ubo_layout_binding.descriptorCount    = 1;
		dynamic_ubo_layout_binding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
		dynamic_ubo_layout_binding.pImmutableSamplers = NULL;

		VkDescriptorSetLayoutBinding sampler_layout_binding = {};
		sampler_layout_binding.binding            = 2;
		sampler_layout_binding.descriptorType     = VK_DESCRIPTOR_TYPE_SAMPLER;
		sampler_layout_binding.descriptorCount    = 1;
		sampler_layout_binding.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
		sampler_layout_binding.pImmutableSamplers = NULL;

		VkDescriptorSetLayoutBinding smsampler_layout_binding = {};
		smsampler_layout_binding.binding            = 3;
		smsampler_layout_binding.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		smsampler_layout_binding.descriptorCount    = 1;
		smsampler_layout_binding.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
		smsampler_layout_binding.pImmutableSamplers = NULL;

		VkDescriptorSetLayoutBinding tex_sampler_layout_binding = {};
		tex_sampler_layout_binding.binding            = 4;
		tex_sampler_layout_binding.descriptorType     = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		tex_sampler_layout_binding.descriptorCount    = VULKAN_MAX_TEXTURE_COUNT;
		tex_sampler_layout_binding.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
		tex_sampler_layout_binding.pImmutableSamplers = NULL;

		VkDescriptorSetLayoutBinding bindings[] = {
			ubo_layout_binding,
			dynamic_ubo_layout_binding,
			sampler_layout_binding,
			smsampler_layout_binding,
			tex_sampler_layout_binding,
		};

		VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {};
		descriptor_set_layout_create_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptor_set_layout_create_info.bindingCount = ARRAY_COUNT(bindings);
		descriptor_set_layout_create_info.pBindings    = bindings;
		VK_CHECK(vkCreateDescriptorSetLayout(vulkan_context.device, &descriptor_set_layout_create_info, NULL, &vulkan_context.descriptor_set_layout));
	}

	// Command pool.
	{
		VkCommandPoolCreateInfo command_pool_create_info = {};
		command_pool_create_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		command_pool_create_info.queueFamilyIndex = vulkan_context.graphics_queue_family;
		VK_CHECK(vkCreateCommandPool(vulkan_context.device, &command_pool_create_info, NULL, &vulkan_context.command_pool));
	}

	// Create shaders.
	{
		const char *textured_static_shaders[] = {"build/textured_static_vertex.spirv", "build/textured_static_fragment.spirv"};
		const char *shadow_map_static_shaders[] = {"build/shadow_map_static_vertex.spirv", "build/shadow_map_static_fragment.spirv"};

		// @TODO: Generate shader table.
		Create_Shader_Request requests[] = {
			{&vulkan_context.shaders[TEXTURED_STATIC_SHADER], textured_static_shaders, ARRAY_COUNT(textured_static_shaders)},
			{&vulkan_context.shaders[SHADOW_MAP_STATIC_SHADER], shadow_map_static_shaders, ARRAY_COUNT(shadow_map_static_shaders)},
		};

		for (s32 i = 0; i < ARRAY_COUNT(requests); i++) {
			requests[i].shaders->module_count = requests[i].shader_module_count;
			requests[i].shaders->modules = allocate_array(&game_state->permanant_arena, Shader_Module, requests[i].shader_module_count);

			for (s32 j = 0; j < requests[i].shader_module_count; j++) {
				// @TODO: Subarena.
				auto spirv = read_entire_file(requests[i].paths[j], &game_state->frame_arena);
				assert(spirv.data);

				VkShaderModuleCreateInfo shader_module_create_info = {};
				shader_module_create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				shader_module_create_info.codeSize = spirv.length;
				shader_module_create_info.pCode    = (u32 *)spirv.data;

				VK_CHECK(vkCreateShaderModule(vulkan_context.device, &shader_module_create_info, NULL, &requests[i].shaders->modules[j].module));

				if (first_occurrence_of(requests[i].paths[j], "vertex.spirv") != NULL) {
					requests[i].shaders->modules[j].stage = VK_SHADER_STAGE_VERTEX_BIT;
				} else if (first_occurrence_of(requests[i].paths[j], "fragment.spirv") != NULL) {
					requests[i].shaders->modules[j].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
				} else {
					_abort("Unknown shader type: %s", requests[i].paths[j]);
				}

				//requests[i].create_info->sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				//requests[i].create_info->stage  = stage;
				//requests[i].create_info->module = requests[i].shaders->modules[j];
				//requests[i].create_info->pName  = "main";
			}
		}
	}

	create_vulkan_display_objects(&game_state->frame_arena);

	// Buffer.
	{
		create_vulkan_buffer(&vulkan_context.gpu_buffer,
		                     &vulkan_context.gpu_memory,
		                     VULKAN_GPU_ALLOCATION_SIZE,
		                     VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
							 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	}

	// Staging buffer.
	{
		create_vulkan_buffer(&vulkan_context.staging_buffer,
		                     &vulkan_context.shared_memory,
		                     VULKAN_SHARED_ALLOCATION_SIZE,
		                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT  | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}

#if 0
	VkMemoryRequirements memory_requirements;
	vkGetBufferMemoryRequirements(vulkan_context.device, *buffer, &memory_requirements);

	allocate_vulkan_memory(memory,  memory_requirements, desired_memory_properties);
	VK_CHECK(vkBindBufferMemory(vulkan_context.device, *buffer, *memory, 0));
	// Vertex buffers.
	{
		VkBuffer vertex_staging_buffer, index_staging_buffer, uniform_buffer;
		VkDeviceMemory vertex_staging_buffer_memory, index_staging_buffer_memory;
		void* data;

		// Vertex buffer.
		create_vulkan_buffer(&vertex_staging_buffer,
		                     &vertex_staging_buffer_memory,
		                     //vulkan_context.vertices.size() * sizeof(Vertex),
		                     ((Model_Asset *)game_state->assets.lookup[0])->meshes[0].vertex_count * sizeof(Vertex),
		                     //MEGABYTE(10),
		                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		VK_CHECK(vkMapMemory(vulkan_context.device, vertex_staging_buffer_memory, 0, ((Model_Asset *)game_state->assets.lookup[0])->meshes[0].vertex_count * sizeof(Vertex), 0, &data));
		memcpy(data, ((Model_Asset *)game_state->assets.lookup[0])->meshes[0].vertices, ((Model_Asset *)game_state->assets.lookup[0])->meshes[0].vertex_count * sizeof(Vertex));
		vkUnmapMemory(vulkan_context.device, vertex_staging_buffer_memory);

		create_vulkan_buffer(&vulkan_context.vertex_buffer,
		                     &vulkan_context.vertex_buffer_memory,
		                     //vulkan_context.vertices.size() * sizeof(Vertex),
		                     ((Model_Asset *)game_state->assets.lookup[0])->meshes[0].vertex_count * sizeof(Vertex),
		                     //MEGABYTE(10),
							 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
							 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		copy_vulkan_buffer(vertex_staging_buffer, vulkan_context.vertex_buffer, ((Model_Asset *)game_state->assets.lookup[0])->meshes[0].vertex_count * sizeof(Vertex));
		vkDestroyBuffer(vulkan_context.device, vertex_staging_buffer, NULL);
		vkFreeMemory(vulkan_context.device, vertex_staging_buffer_memory, NULL);

		// Index buffer.
		create_vulkan_buffer(&index_staging_buffer,
		                     &index_staging_buffer_memory,
		                     //vulkan_context.indices.size() * sizeof(u32),
		                     ((Model_Asset *)game_state->assets.lookup[0])->meshes[0].index_count * sizeof(u32),
		                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		VK_CHECK(vkMapMemory(vulkan_context.device, index_staging_buffer_memory, 0, ((Model_Asset *)game_state->assets.lookup[0])->meshes[0].index_count * sizeof(u32), 0, &data));
		memcpy(data, ((Model_Asset *)game_state->assets.lookup[0])->meshes[0].indices, ((Model_Asset *)game_state->assets.lookup[0])->meshes[0].index_count * sizeof(u32));
		vkUnmapMemory(vulkan_context.device, index_staging_buffer_memory);

		create_vulkan_buffer(&vulkan_context.index_buffer,
		                     &vulkan_context.index_buffer_memory,
		                     //vulkan_context.indices.size() * sizeof(u32),
		                     ((Model_Asset *)game_state->assets.lookup[0])->meshes[0].index_count * sizeof(u32),
							 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
							 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		copy_vulkan_buffer(index_staging_buffer, vulkan_context.index_buffer, ((Model_Asset *)game_state->assets.lookup[0])->meshes[0].index_count * sizeof(u32));
		vkDestroyBuffer(vulkan_context.device, index_staging_buffer, NULL);
		vkFreeMemory(vulkan_context.device, index_staging_buffer_memory, NULL);

		// Uniform buffers.
		for (s32 i = 0; i < vulkan_context.num_swapchain_images; i++) {
			create_vulkan_buffer(&vulkan_context.uniform_buffers[i],
			                     &vulkan_context.uniform_buffers_memory[i],
			                     sizeof(UBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		}

		create_vulkan_buffer(&vulkan_context.smb,
							 &vulkan_context.smbm,
							 sizeof(shadow_map_ubo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
							 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		/*
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = buffer_size;
		bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VK_CHECK(vkCreateBuffer(vulkan_context.device, &bufferInfo, NULL, &vulkan_context.vertex_buffer));

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(vulkan_context.device, vulkan_context.vertex_buffer, &memRequirements);
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(vulkan_context.physical_device, &memProperties);
		s32 memType = -1;
		s32 properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		for (s32 i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((memRequirements.memoryTypeBits & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				memType = i;
				break;
			}
		}
		if (memType < 0) {
			_abort("Failed to find suitable GPU memory type");
		}

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = memType;
		VK_CHECK(vkAllocateMemory(vulkan_context.device, &allocInfo, NULL, &vulkan_context.vertex_buffer_memory));
		vkBindBufferMemory(vulkan_context.device, vulkan_context.vertex_buffer, vulkan_context.vertex_buffer_memory, 0);

		void* data;
		VK_CHECK(vkMapMemory(vulkan_context.device, vulkan_context.vertex_buffer_memory, 0, buffer_size, 0, &data));
		memcpy(data, vertices, (size_t)buffer_size);
		vkUnmapMemory(vulkan_context.device, vulkan_context.vertex_buffer_memory);
		*/
		/*
		auto [ staging_buffer, staging_buffer_memory ] = make_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		void* data;
		VK_CHECK(vkMapMemory(vulkan_context.device, staging_buffer_memory, 0, buffer_size, 0, &data));
		memcpy(data, vertices, (size_t)buffer_size);
		vkUnmapMemory(vk_context.device, staging_buffer_memory);

		auto res = make_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		vk_context.vertex_buffer = res.buffer;
		vk_context.vertex_buffer_memory = res.buffer_memory;

		copy_buffer(staging_buffer, vk_context.vertex_buffer, buffer_size);

		vkDestroyBuffer(vk_context.device, staging_buffer, NULL);
		vkFreeMemory(vk_context.device, staging_buffer_memory, NULL);
		*/
	}
#endif

	// Semaphore.
	{
		VkSemaphoreCreateInfo semaphore_create_info = {};
		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			VK_CHECK(vkCreateSemaphore(vulkan_context.device, &semaphore_create_info, NULL, &vulkan_context.image_available_semaphores[i]));
			VK_CHECK(vkCreateSemaphore(vulkan_context.device, &semaphore_create_info, NULL, &vulkan_context.render_finished_semaphores[i]));
			VK_CHECK(vkCreateFence(vulkan_context.device, &fenceInfo, NULL, &vulkan_context.inFlightFences[i]));
        }
        /*
		VkSemaphoreCreateInfo semaphore_create_info = {};
		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		VK_CHECK(vkCreateSemaphore(vulkan_context.device, &semaphore_create_info, NULL, &vulkan_context.image_available_semaphore));
		VK_CHECK(vkCreateSemaphore(vulkan_context.device, &semaphore_create_info, NULL, &vulkan_context.render_finished_semaphore));
		*/
	}

	// Descriptor pool.
	{
		VkDescriptorPoolSize ubo_pool_size;
		ubo_pool_size.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		ubo_pool_size.descriptorCount = vulkan_context.num_swapchain_images + 6;

		VkDescriptorPoolSize dynamic_ubo_pool_size;
		dynamic_ubo_pool_size.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		dynamic_ubo_pool_size.descriptorCount = 1;

		VkDescriptorPoolSize sampler_pool_size;
		sampler_pool_size.type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		sampler_pool_size.descriptorCount = vulkan_context.num_swapchain_images + 6;

		VkDescriptorPoolSize pool_sizes[] = {
			ubo_pool_size,
			dynamic_ubo_pool_size,
			sampler_pool_size,
		};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.poolSizeCount = ARRAY_COUNT(pool_sizes);
		pool_info.pPoolSizes    = pool_sizes;
		pool_info.maxSets       = vulkan_context.num_swapchain_images + 6;

		VK_CHECK(vkCreateDescriptorPool(vulkan_context.device, &pool_info, NULL, &vulkan_context.descriptor_pool));
	}

	ubo_offset = align_to(VULKAN_UNIFORM_SEGMENT_OFFSET, vulkan_context.minimum_uniform_buffer_offset_alignment);
	sm_ubo_offset = align_to(ubo_offset + sizeof(UBO) * 3 + (256 * 3), vulkan_context.minimum_uniform_buffer_offset_alignment);
	dubo_offset = align_to(sm_ubo_offset + sizeof(shadow_map_ubo), vulkan_context.minimum_uniform_buffer_offset_alignment);

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

	//do_ds();

	// @TODO: Try to use a seperate queue family for transfer operations so that it can be parallelized.

#if 0
	// Vertex buffers.
	{
		VkDeviceSize buffer_size = sizeof(vertices);
		auto [ staging_buffer, staging_buffer_memory ] = make_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		void* data;
		VK_DIE_IF_ERROR(vkMapMemory, vk_context.device, staging_buffer_memory, 0, buffer_size, 0, &data);
		memcpy(data, vertices, (size_t)buffer_size);
		vkUnmapMemory(vk_context.device, staging_buffer_memory);

		auto res = make_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		vk_context.vertex_buffer = res.buffer;
		vk_context.vertex_buffer_memory = res.buffer_memory;

		copy_buffer(staging_buffer, vk_context.vertex_buffer, buffer_size);

		vkDestroyBuffer(vk_context.device, staging_buffer, NULL);
		vkFreeMemory(vk_context.device, staging_buffer_memory, NULL);
	}

	// Index buffers.
	{
		VkDeviceSize buffer_size = sizeof(indices[0]) * ARRAY_COUNT(indices);
		auto [ staging_buffer, staging_buffer_memory ] = make_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		void* data;
		VK_DIE_IF_ERROR(vkMapMemory, vk_context.device, staging_buffer_memory, 0, buffer_size, 0, &data);
		memcpy(data, indices, (size_t)buffer_size);
		vkUnmapMemory(vk_context.device, staging_buffer_memory);

		auto res = make_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		vk_context.index_buffer = res.buffer;
		vk_context.index_buffer_memory = res.buffer_memory;

		copy_buffer(staging_buffer, vk_context.index_buffer, buffer_size);

		vkDestroyBuffer(vk_context.device, staging_buffer, NULL);
		vkFreeMemory(vk_context.device, staging_buffer_memory, NULL);
	}

	// Uniform buffers
	{
		VkDeviceSize buffer_size = sizeof(Uniform_Buffer_Object);
		auto res = make_buffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		vk_context.uniform_buffer = res.buffer;
		vk_context.uniform_buffer_memory = res.buffer_memory;
	}

	// Descriptor pool.
	{
		VkDescriptorPoolSize pool_size = {};
		pool_size.type             =  VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pool_size.descriptorCount  =  1;

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType          =  VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.poolSizeCount  =  1;
		pool_info.pPoolSizes     =  &pool_size;
		pool_info.maxSets        =  1;

		VK_DIE_IF_ERROR(vkCreateDescriptorPool, vk_context.device, &pool_info, NULL, &vk_context.descriptor_pool);
	}

	// Descriptor set.
	{
		VkDescriptorSetLayout layouts[] = {vk_context.descriptor_set_layout};
		VkDescriptorSetAllocateInfo ai = {};
		ai.sType               =  VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		ai.descriptorPool      =  vk_context.descriptor_pool;
		ai.descriptorSetCount  =  1;
		ai.pSetLayouts         =  layouts;
		VK_DIE_IF_ERROR(vkAllocateDescriptorSets, vk_context.device, &ai, &vk_context.descriptor_set);

		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer  =  vk_context.uniform_buffer;
		bufferInfo.offset  =  0;
		bufferInfo.range   =  sizeof(Uniform_Buffer_Object);

		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType            =  VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet           =  vk_context.descriptor_set;
		descriptorWrite.dstBinding       =  0;
		descriptorWrite.dstArrayElement  =  0;
		descriptorWrite.descriptorType   =  VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount  =  1;
		descriptorWrite.pBufferInfo = &bufferInfo;
		descriptorWrite.pImageInfo = NULL;
		descriptorWrite.pTexelBufferView = NULL;

		vkUpdateDescriptorSets(vk_context.device, 1, &descriptorWrite, 0, NULL);
	}

	vk_context.command_buffers = make_command_buffers(vk_context.swapchain_context, vk_context.device, vk_context.command_pool);

	// Semaphore.
	{
		VkSemaphoreCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		VK_DIE_IF_ERROR(vkCreateSemaphore, vk_context.device, &ci, NULL, &vk_context.image_available_semaphore);
		VK_DIE_IF_ERROR(vkCreateSemaphore, vk_context.device, &ci, NULL, &vk_context.render_finished_semaphore);
	}
#endif
}

const M4 shadow_map_clip_space_bias = {
	0.5, 0.0, 0.0, 0.5,
	0.0, 0.5, 0.0, 0.5,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0
};

#include <chrono>
void render(Game_State *game_state) {
	vkWaitForFences(vulkan_context.device, 1, &vulkan_context.inFlightFences[vulkan_context.currentFrame], true, UINT64_MAX);
	vkResetFences(vulkan_context.device, 1, &vulkan_context.inFlightFences[vulkan_context.currentFrame]);

    u32 image_index;
    VK_CHECK(vkAcquireNextImageKHR(vulkan_context.device, vulkan_context.swapchain, UINT64_MAX, vulkan_context.image_available_semaphores[vulkan_context.currentFrame], NULL, &image_index));

	VkSemaphore wait_semaphores[] = {vulkan_context.image_available_semaphores[vulkan_context.currentFrame]};
	VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSemaphore signal_semaphores[] = {vulkan_context.render_finished_semaphores[vulkan_context.currentFrame]};

	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	auto model = m4_identity();
	UBO scene_ubo;
	//Dynamic_Scene_UBO dynamic_scene_ubo;
	//ubo.model = m4_identity();//glm::transpose(glm::rotate(glm::mat4(1.0f), 0 * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
	//ubo.view = glm::lookAt(glm::vec3(3.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//ubo.projection = glm::perspective(glm::radians(45.0f), vulkan_context.swapchain_image_extent.width / (float)vulkan_context.swapchain_image_extent.height,0.1f, 10.0f);

	//ubo.view = game_state->camera.view_matrix;//(look_at({2.0f, 2.0f, 2.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}));
	//ubo.view = (look_at({2.0f, 2.0f, 2.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}));
	//ubo.projection = perspective_projection(90.0f, vulkan_context.swapchain_image_extent.width / (float)vulkan_context.swapchain_image_extent.height);
	dynamic_scene_ubo[0].world_to_clip_space = perspective_projection(90.0f, vulkan_context.swapchain_image_extent.width / (float)vulkan_context.swapchain_image_extent.height) * game_state->camera.view_matrix * model;

	//shadow_map_ubo.model = m4_identity();
	//shadow_map_ubo.view = look_at(V3{2.0f, 2.0f, 2.0f}, V3{0.0f, 0.0f, 0.0f}, V3{0.0f, 0.0f, 1.0f});
	shadow_map_ubo.world_to_clip_space = orthographic_projection(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 10.0f) * look_at(V3{2.0f, 2.0f, 2.0f}, V3{0.0f, 0.0f, 0.0f}, V3{0.0f, 0.0f, 1.0f}) * m4_identity();
	//shadow_map_ubo.projection = perspective_projection(90.0f, 1.0f);

	//ubo.light_model = shadow_map_ubo.model;
	//ubo.light_view = shadow_map_ubo.view;
	//ubo.light_projection = shadow_map_ubo.projection;
	dynamic_scene_ubo[0].world_to_shadow_map_clip_space = shadow_map_clip_space_bias * shadow_map_ubo.world_to_clip_space * model;

	void* data;
	vkMapMemory(vulkan_context.device, vulkan_context.shared_memory, align_to(ubo_offset + (sizeof(UBO) * image_index), vulkan_context.minimum_uniform_buffer_offset_alignment), sizeof(scene_ubo), 0, &data);
	memcpy(data, &scene_ubo, sizeof(scene_ubo));
	vkUnmapMemory(vulkan_context.device, vulkan_context.shared_memory);

	vkMapMemory(vulkan_context.device, vulkan_context.shared_memory, sm_ubo_offset, sizeof(shadow_map_ubo), 0, &data);
	memcpy(data, &shadow_map_ubo, sizeof(shadow_map_ubo));
	vkUnmapMemory(vulkan_context.device, vulkan_context.shared_memory);

	vkMapMemory(vulkan_context.device, vulkan_context.shared_memory, dubo_offset, sizeof(dynamic_scene_ubo), 0, &data);
	memcpy(data, &dynamic_scene_ubo, sizeof(dynamic_scene_ubo));
	vkUnmapMemory(vulkan_context.device, vulkan_context.shared_memory);

	{
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
		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = ubo_offset; // Optional
		copyRegion.dstOffset = ubo_offset; // Optional
		//copyRegion.size = (sm_ubo_offset + sizeof(shadow_map_ubo)) - ubo_offset;
		copyRegion.size = (dubo_offset + sizeof(dynamic_scene_ubo)) - ubo_offset;
		//printf("copy %lu %lu %lu\n", VULKAN_GPU_ALLOCATION_SIZE, VULKAN_SHARED_ALLOCATION_SIZE, ubo_offset + copyRegion.size);
		vkCmdCopyBuffer(command_buffer, vulkan_context.staging_buffer, vulkan_context.gpu_buffer, 1, &copyRegion);
		vkEndCommandBuffer(command_buffer);
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &command_buffer;

		vkQueueSubmit(vulkan_context.graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(vulkan_context.graphics_queue); // @TODO: Use a fence?
		//vkFreeCommandBuffers(vulkan_context.device, vulkan_context.command_pool, 1, &command_buffer);
	}

	VkSubmitInfo submit_info = {};
	submit_info.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.waitSemaphoreCount   = 1;
	submit_info.pWaitSemaphores      = wait_semaphores;
	submit_info.pWaitDstStageMask    = wait_stages;
	submit_info.commandBufferCount   = 1;
	submit_info.pCommandBuffers      = &vulkan_context.command_buffers[image_index];
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores    = signal_semaphores;

	VK_CHECK(vkQueueSubmit(vulkan_context.graphics_queue, 1, &submit_info, vulkan_context.inFlightFences[vulkan_context.currentFrame]));

	VkSwapchainKHR swapchains[] = {vulkan_context.swapchain};

	VkPresentInfoKHR present_info = {};
	present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores    = signal_semaphores;
	present_info.swapchainCount     = 1;
	present_info.pSwapchains        = swapchains;
	present_info.pImageIndices      = &image_index;

	VK_CHECK(vkQueuePresentKHR(vulkan_context.present_queue, &present_info));

	vulkan_context.currentFrame = (vulkan_context.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void cleanup_renderer() {
	vkDeviceWaitIdle(vulkan_context.device);

	if (debug) {
		vkDestroyDebugUtilsMessengerEXT(vulkan_context.instance, vulkan_context.debug_messenger, NULL);
	}

	for (s32 i = 0; i < SHADER_COUNT; i++) {
		for (s32 j = 0; j < vulkan_context.shaders[i].module_count; j++) {
			vkDestroyShaderModule(vulkan_context.device, vulkan_context.shaders[i].modules[j].module, NULL);
		}
	}

	for (s32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
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
	vkDestroyImageView(vulkan_context.device, vulkan_context.textureImageView, NULL);

	vkDestroyImage(vulkan_context.device, vulkan_context.textureImage, NULL);
	vkFreeMemory(vulkan_context.device, vulkan_context.textureImageMemory, NULL);

	vkDestroyDescriptorPool(vulkan_context.device, vulkan_context.descriptor_pool, NULL);
	vkDestroyDescriptorSetLayout(vulkan_context.device, vulkan_context.descriptor_set_layout, NULL);

	//vkDestroyBuffer(vulkan_context.device, vulkan_context.vertex_buffer, NULL);
	//vkFreeMemory(vulkan_context.device, vulkan_context.vertex_buffer_memory, NULL);

	//vkDestroyBuffer(vulkan_context.device, vulkan_context.index_buffer, NULL);
	//vkFreeMemory(vulkan_context.device, vulkan_context.index_buffer_memory, NULL);

	vkDestroyCommandPool(vulkan_context.device, vulkan_context.command_pool, NULL);

	vkDeviceWaitIdle(vulkan_context.device);

	vkDestroyDevice(vulkan_context.device, NULL);
	vkDestroySurfaceKHR(vulkan_context.instance, vulkan_context.surface, NULL);

	cleanup_platform_display();

	vkDestroyInstance(vulkan_context.instance, NULL); // On X11, the Vulkan instance must be destroyed after the display resources are destroyed.
}
