#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_xlib.h>

#define MAX_FRAMES_IN_FLIGHT 2

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
	VkPipeline               pipeline;
	VkPipelineLayout         pipeline_layout;
	VkDescriptorSetLayout    descriptor_set_layout;
	VkCommandPool            command_pool;
	VkCommandBuffer          command_buffers[3];
	VkSemaphore              image_available_semaphores[MAX_FRAMES_IN_FLIGHT];
	VkSemaphore              render_finished_semaphores[MAX_FRAMES_IN_FLIGHT];
	VkFence                  inFlightFences[MAX_FRAMES_IN_FLIGHT];
    u32                      currentFrame = 0;
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
		log_type = CATASTROPHIC_ERROR_LOG;
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

struct Vertex {
	V2 position;
	V3 color;
};

void create_vulkan_display_objects() {
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
		auto swapchain_images = allocate_array(&temporary_memory_arena, VkImage, vulkan_context.num_swapchain_images);
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
		VkAttachmentDescription color_attachment = {};
		color_attachment.format         = vulkan_context.surface_format.format;
		color_attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
		color_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_attachment_reference = {};
		color_attachment_reference.attachment = 0;
		color_attachment_reference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass_description = {};
		subpass_description.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass_description.colorAttachmentCount = 1;
		subpass_description.pColorAttachments    = &color_attachment_reference;

		VkSubpassDependency subpass_dependency = {};
		subpass_dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
		subpass_dependency.dstSubpass    = 0;
		subpass_dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpass_dependency.srcAccessMask = 0;
		subpass_dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo render_pass_create_info = {};
		render_pass_create_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_create_info.attachmentCount = 1;
		render_pass_create_info.pAttachments    = &color_attachment;
		render_pass_create_info.subpassCount    = 1;
		render_pass_create_info.pSubpasses      = &subpass_description;
		render_pass_create_info.dependencyCount = 1;
		render_pass_create_info.pDependencies   = &subpass_dependency;

		VK_CHECK(vkCreateRenderPass(vulkan_context.device, &render_pass_create_info, NULL, &vulkan_context.render_pass));
	}

	// Pipeline.
	{
		//Memory_Arena load_shaders = mem_make_arena();
		auto vertex_spirv = read_entire_file("build/vertex.spirv", &temporary_memory_arena);
		assert(vertex_spirv.data);
		auto fragment_spirv = read_entire_file("build/fragment.spirv", &temporary_memory_arena);
		assert(fragment_spirv.data);

		auto create_shader_module = [](String_Result binary)
		{
			VkShaderModuleCreateInfo shader_module_create_info = {};
			shader_module_create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shader_module_create_info.codeSize = binary.length;
			shader_module_create_info.pCode    = (u32 *)binary.data;

			VkShaderModule shader_module;
			VK_CHECK(vkCreateShaderModule(vulkan_context.device, &shader_module_create_info, NULL, &shader_module));

			return shader_module;
		};
		auto vertex_shader_module = create_shader_module(vertex_spirv);
		auto fragment_shader_module = create_shader_module(fragment_spirv);

		DEFER(vkDestroyShaderModule(vulkan_context.device, vertex_shader_module, NULL));
		DEFER(vkDestroyShaderModule(vulkan_context.device, fragment_shader_module, NULL));

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

#if 0
		VkVertexInputBindingDescription vertex_input_binding_description = {};
		vertex_input_binding_description.binding   = 0;
		vertex_input_binding_description.stride    = sizeof(Vertex);
		vertex_input_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription vertex_input_attribute_descriptions[2];
		vertex_input_attribute_descriptions[0].binding  = 0;
		vertex_input_attribute_descriptions[0].location = 0;
		vertex_input_attribute_descriptions[0].format   = VK_FORMAT_R32G32_SFLOAT;
		vertex_input_attribute_descriptions[0].offset   = offsetof(Vertex, position);

		vertex_input_attribute_descriptions[1].binding  = 0;
		vertex_input_attribute_descriptions[1].location = 1;
		vertex_input_attribute_descriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
		vertex_input_attribute_descriptions[1].offset   = offsetof(Vertex, color);

		VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {};
		vertex_input_state_create_info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_state_create_info.vertexBindingDescriptionCount   = 1;
		vertex_input_state_create_info.pVertexBindingDescriptions      = &vertex_input_binding_description;
		vertex_input_state_create_info.vertexAttributeDescriptionCount = ARRAY_COUNT(vertex_input_attribute_descriptions);
		vertex_input_state_create_info.pVertexAttributeDescriptions    = vertex_input_attribute_descriptions;
#endif

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;

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
		rasterization_state_create_info.frontFace               = VK_FRONT_FACE_CLOCKWISE;
		rasterization_state_create_info.depthBiasEnable         = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisample_state_create_info = {};
		multisample_state_create_info.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_state_create_info.sampleShadingEnable   = VK_FALSE;
		multisample_state_create_info.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
		//multisample_state_create_info.minSampleShading      = 1.0f;
		//multisample_state_create_info.alphaToCoverageEnable = VK_FALSE;
		//multisample_state_create_info.alphaToOneEnable      = VK_FALSE;

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

		// Use pipeline layout to specify uniform layout.
		VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
		pipeline_layout_create_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.setLayoutCount         = 0;
		pipeline_layout_create_info.pushConstantRangeCount = 0;
		/*
		pipeline_layout_create_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.setLayoutCount         = 1;
		pipeline_layout_create_info.pSetLayouts            = &vulkan_context.descriptor_set_layout;
		pipeline_layout_create_info.pushConstantRangeCount = 0;
		pipeline_layout_create_info.pPushConstantRanges    = NULL;
		*/

		VK_CHECK(vkCreatePipelineLayout(vulkan_context.device, &pipeline_layout_create_info, NULL, &vulkan_context.pipeline_layout));

		VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {};
		graphics_pipeline_create_info.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		graphics_pipeline_create_info.stageCount          = 2;
		graphics_pipeline_create_info.pStages             = pipeline_shader_stage_create_info;
		graphics_pipeline_create_info.pVertexInputState   = &vertexInputInfo;
		graphics_pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
		graphics_pipeline_create_info.pViewportState      = &viewport_state_create_info;
		graphics_pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
		graphics_pipeline_create_info.pMultisampleState   = &multisample_state_create_info;
		graphics_pipeline_create_info.pColorBlendState    = &color_blend_state_create_info;
		graphics_pipeline_create_info.layout              = vulkan_context.pipeline_layout;
		graphics_pipeline_create_info.renderPass          = vulkan_context.render_pass;
		graphics_pipeline_create_info.subpass             = 0;
		//graphics_pipeline_create_info.basePipelineIndex   = -1;
		//graphics_pipeline_create_info.basePipelineHandle  = VK_NULL_HANDLE;
		//graphics_pipeline_create_info.basePipelineIndex   = -1;

		VK_CHECK(vkCreateGraphicsPipelines(vulkan_context.device, NULL, 1, &graphics_pipeline_create_info, NULL, &vulkan_context.pipeline));
	}

	// Framebuffers.
	{
		for (s32 i = 0; i < vulkan_context.num_swapchain_images; ++i) {
			VkImageView attachments[] = {vulkan_context.swapchain_image_views[i]};

			VkFramebufferCreateInfo framebuffer_create_info = {};
			framebuffer_create_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebuffer_create_info.renderPass      = vulkan_context.render_pass;
			framebuffer_create_info.attachmentCount = 1;
			framebuffer_create_info.pAttachments    = attachments;
			framebuffer_create_info.width           = vulkan_context.swapchain_image_extent.width;
			framebuffer_create_info.height          = vulkan_context.swapchain_image_extent.height;
			framebuffer_create_info.layers          = 1;

			VK_CHECK(vkCreateFramebuffer(vulkan_context.device, &framebuffer_create_info, NULL, &vulkan_context.framebuffers[i]));
		}
	}
}

void create_vulkan_command_buffers() {
	//Array<VkCommandBuffer> command_buffers{sc.framebuffers.size};

	VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
	command_buffer_allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_info.commandPool        = vulkan_context.command_pool;
	command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_allocate_info.commandBufferCount = vulkan_context.num_swapchain_images;
	VK_CHECK(vkAllocateCommandBuffers(vulkan_context.device, &command_buffer_allocate_info, vulkan_context.command_buffers));

	// Fill command buffers.
	for (u32 i = 0; i < vulkan_context.num_swapchain_images; i++) {
		VkCommandBufferBeginInfo command_buffer_begin_info = {};
		command_buffer_begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		command_buffer_begin_info.flags            = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		VK_CHECK(vkBeginCommandBuffer(vulkan_context.command_buffers[i], &command_buffer_begin_info));

		VkClearValue clear_color = {0.5f, 0.5f, 0.5f, 1.0f};
		VkRenderPassBeginInfo render_pass_begin_info = {};
		render_pass_begin_info.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_info.renderPass        = vulkan_context.render_pass;
		render_pass_begin_info.framebuffer       = vulkan_context.framebuffers[i];
		render_pass_begin_info.renderArea.offset = {0, 0};
		render_pass_begin_info.renderArea.extent = vulkan_context.swapchain_image_extent;
		render_pass_begin_info.clearValueCount   = 1;
		render_pass_begin_info.pClearValues      = &clear_color;

		vkCmdBeginRenderPass(vulkan_context.command_buffers[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(vulkan_context.command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan_context.pipeline);

		/*
		VkBuffer vertex_buffers[] = {vulkan_context.vertex_buffer};
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(vulkan_context.command_buffers[i], 0, 1, vertex_buffers, offsets);
		vkCmdBindIndexBuffer(vulkan_context.command_buffers[i], vulkan_context.index_buffer, 0, VK_INDEX_TYPE_UINT16);
		vkCmdBindDescriptorSets(vulkan_context.command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan_context.pipeline_layout, 0, 1, &vulkan_context.descriptor_set, 0, NULL);
		*/

		vkCmdDraw(vulkan_context.command_buffers[i], 3, 1, 0, 0);
		//vkCmdDrawIndexed(vulkan_context.command_buffers[i], ARRAY_COUNT(indices), 1, 0, 0, 0);

		vkCmdEndRenderPass(vulkan_context.command_buffers[i]);

		VK_CHECK(vkEndCommandBuffer(vulkan_context.command_buffers[i]));
	}
}

u8 strings_subset_test(const char **, s32, const char **, s32);

void initialize_vulkan() {
	Library_Handle vulkan_library = open_shared_library("dependencies/vulkan/1.1.106.0/lib/libvulkan.so");

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
	name = (PFN_##name)load_shared_library_function(vulkan_library, (const char *)#name);\
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
			if (!physical_device_features.geometryShader) {
				//debug_print("Skipping physical device because it is not a discrete gpu.\n");
				continue;
			}

			u32 num_available_device_extensions = 0;
			VK_CHECK(vkEnumerateDeviceExtensionProperties(physical_devices[i], NULL, &num_available_device_extensions, NULL));
			//debug_print("\tNumber of available device extensions: %u\n", num_available_device_extensions);
			auto available_device_extensions = allocate_array(&temporary_memory_arena, VkExtensionProperties, num_available_device_extensions);
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
			auto available_surface_formats = allocate_array(&temporary_memory_arena, VkSurfaceFormatKHR, num_available_surface_formats);
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
			auto available_present_modes = allocate_array(&temporary_memory_arena, VkPresentModeKHR, num_available_present_modes);
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
			auto queue_families = allocate_array(&temporary_memory_arena, VkQueueFamilyProperties, num_queue_families);
			vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[i], &num_queue_families, queue_families);

			//debug_print("\tFound %u queue families.\n", num_queue_families);

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

		auto device_queue_create_info = allocate_array(&temporary_memory_arena, VkDeviceQueueCreateInfo, num_unique_queue_families);
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

#if 0
	// Descriptor set layout.
	{
		VkDescriptorSetLayoutBinding descriptor_set_layout_binding = {};
		descriptor_set_layout_binding.binding            = 0;
		descriptor_set_layout_binding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptor_set_layout_binding.descriptorCount    = 1;
		descriptor_set_layout_binding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
		descriptor_set_layout_binding.pImmutableSamplers = NULL;

		VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {};
		descriptor_set_layout_create_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptor_set_layout_create_info.bindingCount = 1;
		descriptor_set_layout_create_info.pBindings    = &descriptor_set_layout_binding;
		VK_CHECK(vkCreateDescriptorSetLayout(vulkan_context.device, &descriptor_set_layout_create_info, NULL, &vulkan_context.descriptor_set_layout));
	}
#endif

	create_vulkan_display_objects();

	// Command pool.
	{
		VkCommandPoolCreateInfo command_pool_create_info = {};
		command_pool_create_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		command_pool_create_info.queueFamilyIndex = vulkan_context.graphics_queue_family;
		VK_CHECK(vkCreateCommandPool(vulkan_context.device, &command_pool_create_info, NULL, &vulkan_context.command_pool));
	}
	
	create_vulkan_command_buffers();

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

#if 0
	// @TODO: Try to use a seperate queue family for transfer operations so that it can be parallelized.

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

void render() {
	vkWaitForFences(vulkan_context.device, 1, &vulkan_context.inFlightFences[vulkan_context.currentFrame], true, UINT64_MAX);
	vkResetFences(vulkan_context.device, 1, &vulkan_context.inFlightFences[vulkan_context.currentFrame]);

    u32 image_index;
    VK_CHECK(vkAcquireNextImageKHR(vulkan_context.device, vulkan_context.swapchain, UINT64_MAX, vulkan_context.image_available_semaphores[vulkan_context.currentFrame], NULL, &image_index));

	VkSemaphore wait_semaphores[] = {vulkan_context.image_available_semaphores[vulkan_context.currentFrame]};
	VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSemaphore signal_semaphores[] = {vulkan_context.render_finished_semaphores[vulkan_context.currentFrame]};

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

void vulkan_cleanup() {
	vkDeviceWaitIdle(vulkan_context.device);

	if (debug) {
		vkDestroyDebugUtilsMessengerEXT(vulkan_context.instance, vulkan_context.debug_messenger, NULL);
	}
	vkDestroyDevice(vulkan_context.device, NULL);
	vkDestroySurfaceKHR(vulkan_context.instance, vulkan_context.surface, NULL);
	vkDestroyInstance(vulkan_context.instance, NULL);

#if 0
	auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vk_context.instance, "vkDestroyDebugReportCallbackEXT");
	if(!vkDestroyDebugReportCallbackEXT)
		_abort("Could not load vkDestroyDebugReportCallbackEXT function.");
	vkDestroyDebugReportCallbackEXT(vk_context.instance, vk_context.debug_callback, NULL);
	vkDestroySemaphore(d, vk_context.render_finished_semaphore, NULL);
	vkDestroySemaphore(d, vk_context.image_available_semaphore, NULL);
	vkDestroyCommandPool(d, vk_context.command_pool, NULL);
	for (auto fb : vk_context.swapchain_context.framebuffers)
		vkDestroyFramebuffer(d, fb, NULL);
	vkDestroyPipeline(d, vk_context.swapchain_context.pipeline, NULL);
	vkDestroyPipelineLayout(d, vk_context.swapchain_context.pipeline_layout, NULL);
	vkDestroyRenderPass(d, vk_context.swapchain_context.render_pass, NULL);
	for (auto iv : vk_context.swapchain_context.image_views)
		vkDestroyImageView(d, iv, NULL);
	vkDestroySwapchainKHR(d, vk_context.swapchain_context.swapchain, NULL);

	vkDestroyDescriptorPool(d, vk_context.descriptor_pool, NULL);
	vkDestroyDescriptorSetLayout(d, vk_context.descriptor_set_layout, NULL);
	vkDestroyBuffer(d, vk_context.uniform_buffer, NULL);
	vkFreeMemory(d, vk_context.uniform_buffer_memory, NULL);
	vkDestroyBuffer(d, vk_context.vertex_buffer, NULL);
	vkFreeMemory(d, vk_context.vertex_buffer_memory, NULL);
	vkDestroyBuffer(d, vk_context.index_buffer, NULL);
	vkFreeMemory(d, vk_context.index_buffer_memory, NULL);
	vkDestroyDevice(d, NULL);
	vkDestroySurfaceKHR(vk_context.instance, vk_context.surface, NULL);
	vkDestroyInstance(vk_context.instance, NULL);
#endif
}

