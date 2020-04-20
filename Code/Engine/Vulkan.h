#pragma once

// @TODO: Switch all enums to match Vulkan name.

#if defined(USE_VULKAN_RENDER_API)

constexpr bool usingVulkanAPI = true;

#define GFX_MAX_FRAMES_IN_FLIGHT 2

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_xlib.h> 

typedef VkFlags GfxShaderStageFlags;
typedef VkShaderStageFlagBits GfxShaderStage;
#define GFX_VERTEX_SHADER_STAGE VK_SHADER_STAGE_VERTEX_BIT
#define GFX_FRAGMENT_SHADER_STAGE VK_SHADER_STAGE_FRAGMENT_BIT
#define GFX_COMPUTE_SHADER_STAGE VK_SHADER_STAGE_COMPUTE_BIT

typedef VkFlags GfxBufferUsageFlags;
typedef VkBufferUsageFlagBits GfxBufferUsage;
#define GFX_TRANSFER_DESTINATION_BUFFER VK_BUFFER_USAGE_TRANSFER_DST_BIT
#define GFX_TRANSFER_SOURCE_BUFFER VK_BUFFER_USAGE_TRANSFER_SRC_BIT
#define GFX_VERTEX_BUFFER VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
#define GFX_INDEX_BUFFER VK_BUFFER_USAGE_INDEX_BUFFER_BIT
#define GFX_UNIFORM_BUFFER VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT

typedef VkFlags GfxColorComponentFlags;
typedef VkColorComponentFlagBits GfxColorComponent;
#define GFX_COLOR_COMPONENT_RED VK_COLOR_COMPONENT_R_BIT
#define GFX_COLOR_COMPONENT_GREEN VK_COLOR_COMPONENT_G_BIT
#define GFX_COLOR_COMPONENT_BLUE VK_COLOR_COMPONENT_B_BIT
#define GFX_COLOR_COMPONENT_ALPHA VK_COLOR_COMPONENT_A_BIT

typedef VkFilter GfxFilter;
#define GFX_LINEAR_FILTER VK_FILTER_LINEAR
#define GFX_NEAREST_FILTER VK_FILTER_NEAREST
#define GFX_CUBIC_FILTER VK_FILTER_CUBIC_IMG

typedef VkFlags GfxImageUsageFlags;
typedef VkImageUsageFlagBits GfxImageUsage;
#define GFX_IMAGE_USAGE_TRANSFER_SRC VK_IMAGE_USAGE_TRANSFER_SRC_BIT
#define GFX_IMAGE_USAGE_TRANSFER_DST VK_IMAGE_USAGE_TRANSFER_DST_BIT
#define GFX_IMAGE_USAGE_SAMPLED VK_IMAGE_USAGE_SAMPLED_BIT
#define GFX_IMAGE_USAGE_STORAGE VK_IMAGE_USAGE_STORAGE_BIT
#define GFX_IMAGE_USAGE_COLOR_ATTACHMENT VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
#define GFX_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
#define GFX_IMAGE_USAGE_TRANSIENT_ATTACHMENT VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
#define GFX_IMAGE_USAGE_INPUT_ATTACHMENT VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
#define GFX_IMAGE_USAGE_SHADING_RATE_IMAGE_NV VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV
#define GFX_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_EXT VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT

typedef VkFlags GfxSampleCountFlags;
typedef VkSampleCountFlagBits GfxSampleCount;
#define GFX_SAMPLE_COUNT_1 VK_SAMPLE_COUNT_1_BIT
#define GFX_SAMPLE_COUNT_2 VK_SAMPLE_COUNT_2_BIT
#define GFX_SAMPLE_COUNT_4 VK_SAMPLE_COUNT_4_BIT
#define GFX_SAMPLE_COUNT_8 VK_SAMPLE_COUNT_8_BIT
#define GFX_SAMPLE_COUNT_16 VK_SAMPLE_COUNT_16_BIT
#define GFX_SAMPLE_COUNT_32 VK_SAMPLE_COUNT_32_BIT
#define GFX_SAMPLE_COUNT_64 VK_SAMPLE_COUNT_64_BIT

typedef VkBlendFactor GfxBlendFactor;
#define GFX_BLEND_FACTOR_SRC_ALPHA VK_BLEND_FACTOR_SRC_ALPHA
#define GFX_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
#define GFX_BLEND_FACTOR_ONE VK_BLEND_FACTOR_ONE
#define GFX_BLEND_FACTOR_ZERO VK_BLEND_FACTOR_ZERO

enum GfxBlendOperation
{
	GFX_BLEND_OP_ADD = VK_BLEND_OP_ADD,
};

enum GfxVertexInputRate
{
	GFX_VERTEX_INPUT_RATE_VERTEX = VK_VERTEX_INPUT_RATE_VERTEX,
	GFX_VERTEX_INPUT_RATE_INSTANCE = VK_VERTEX_INPUT_RATE_INSTANCE,
};

typedef VkFormat GfxFormat;
#define GFX_FORMAT_R32G32B32_SFLOAT VK_FORMAT_R32G32B32_SFLOAT
#define GFX_FORMAT_R32_UINT VK_FORMAT_R32_UINT
#define GFX_FORMAT_D32_SFLOAT_S8_UINT VK_FORMAT_D32_SFLOAT_S8_UINT
#define GFX_FORMAT_R8G8B8A8_UNORM VK_FORMAT_R8G8B8A8_UNORM
#define GFX_FORMAT_D16_UNORM VK_FORMAT_D16_UNORM
#define GFX_FORMAT_UNDEFINED VK_FORMAT_UNDEFINED

typedef VkDescriptorType GfxDescriptorType;
#define GFX_DESCRIPTOR_TYPE_UNIFORM_BUFFER VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
#define GFX_DESCRIPTOR_TYPE_DYNAMIC_UNIFORM_BUFFER VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
#define GFX_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
#define GFX_DESCRIPTOR_TYPE_SAMPLER VK_DESCRIPTOR_TYPE_SAMPLER
#define GFX_DESCRIPTOR_TYPE_SAMPLED_IMAGE VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE

enum GfxPipelineTopology
{
	GFX_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	GFX_PRIMITIVE_TOPOLOGY_LINE_LIST = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
};

enum GfxCompareOperation
{
	GFX_COMPARE_OP_NEVER = VK_COMPARE_OP_NEVER,
	GFX_COMPARE_OP_LESS = VK_COMPARE_OP_LESS,
	GFX_COMPARE_OP_EQUAL = VK_COMPARE_OP_EQUAL,
	GFX_COMPARE_OP_LESS_OR_EQUAL = VK_COMPARE_OP_LESS_OR_EQUAL,
	GFX_COMPARE_OP_GREATER = VK_COMPARE_OP_GREATER,
	GFX_COMPARE_OP_NOT_EQUAL = VK_COMPARE_OP_NOT_EQUAL,
	GFX_COMPARE_OP_GREATER_OR_EQUAL = VK_COMPARE_OP_GREATER_OR_EQUAL,
	GFX_COMPARE_OP_ALWAYS = VK_COMPARE_OP_ALWAYS,
};

enum GfxDynamicPipelineState
{
	GFX_DYNAMIC_PIPELINE_STATE_VIEWPORT = VK_DYNAMIC_STATE_VIEWPORT,
	GFX_DYNAMIC_PIPELINE_STATE_SCISSOR = VK_DYNAMIC_STATE_SCISSOR,
	GFX_DYNAMIC_PIPELINE_STATE_LINE_WIDTH = VK_DYNAMIC_STATE_LINE_WIDTH,
	GFX_DYNAMIC_PIPELINE_STATE_DEPTH_BIAS = VK_DYNAMIC_STATE_DEPTH_BIAS,
	GFX_DYNAMIC_PIPELINE_STATE_BLEND_CONSTANTS = VK_DYNAMIC_STATE_BLEND_CONSTANTS,
	GFX_DYNAMIC_PIPELINE_STATE_DEPTH_BOUNDS = VK_DYNAMIC_STATE_DEPTH_BOUNDS,
	GFX_DYNAMIC_PIPELINE_STATE_STENCIL_COMPARE_MASK = VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
	GFX_DYNAMIC_PIPELINE_STATE_STENCIL_WRITE_MASK = VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
	GFX_DYNAMIC_PIPELINE_STATE_STENCIL_REFERENCE = VK_DYNAMIC_STATE_STENCIL_REFERENCE,
};

typedef VkImageLayout GfxImageLayout;
#define GFX_IMAGE_LAYOUT_UNDEFINED VK_IMAGE_LAYOUT_UNDEFINED
#define GFX_IMAGE_LAYOUT_GENERAL VK_IMAGE_LAYOUT_GENERAL
#define GFX_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
#define GFX_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
#define GFX_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
#define GFX_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
#define GFX_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
#define GFX_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
#define GFX_IMAGE_LAYOUT_PREINITIALIZED VK_IMAGE_LAYOUT_PREINITIALIZED
#define GFX_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL
#define GFX_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL
#define GFX_IMAGE_LAYOUT_PRESENT_SRC VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
#define GFX_IMAGE_LAYOUT_SHARED_PRESENT VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR
#define GFX_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV
#define GFX_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT

typedef VkPipelineBindPoint GfxPipelineBindPoint;
#define GFX_GRAPHICS_PIPELINE_BIND_POINT VK_PIPELINE_BIND_POINT_GRAPHICS
#define GFX_COMPUTE_PIPELINE_BIND_POINT VK_PIPELINE_BIND_POINT_COMPUTE

enum GfxSamplerAddressMode
{
	GFX_SAMPLER_ADDRESS_MODE_REPEAT = VK_SAMPLER_ADDRESS_MODE_REPEAT,
	GFX_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
	GFX_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
	GFX_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
	GFX_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE,
};

enum GfxBorderColor
{
	GFX_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
	GFX_BORDER_COLOR_INT_TRANSPARENT_BLACK = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK,
	GFX_BORDER_COLOR_FLOAT_OPAQUE_BLACK = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
	GFX_BORDER_COLOR_INT_OPAQUE_BLACK = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
	GFX_BORDER_COLOR_FLOAT_OPAQUE_WHITE = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
	GFX_BORDER_COLOR_INT_OPAQUE_WHITE = VK_BORDER_COLOR_INT_OPAQUE_WHITE,
};

enum GfxCommandQueueType
{
	GFX_GRAPHICS_COMMAND_QUEUE,
	GFX_COMPUTE_COMMAND_QUEUE,
	GFX_TRANSFER_COMMAND_QUEUE,
};

enum GfxMemoryType
{
	GFX_DEVICE_MEMORY = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	GFX_HOST_MEMORY = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
};

struct VulkanSamplerFilter
{
	VkFilter min;
	VkFilter mag;
	VkSamplerMipmapMode mipmap;
};

typedef VkBuffer GfxBuffer;
typedef VkSemaphore GfxSemaphore;
typedef VkDescriptorSetLayout GfxDescriptorSetLayout;
typedef VkDescriptorSet GfxDescriptorSet;
typedef VkFence GfxFence;
typedef VkCommandBuffer GfxCommandBuffer;
typedef VkCommandPool GfxCommandPool;
typedef VkDeviceMemory GfxMemory;
typedef VkSampler GfxSampler;
typedef VkShaderModule GfxShaderModule;
typedef VkRenderPass GfxRenderPass;
typedef VkDescriptorPool GfxDescriptorPool;
typedef VkFramebuffer GfxFramebuffer;
typedef VkSwapchainKHR GfxSwapchain;
typedef VkMemoryRequirements GfxMemoryRequirements;
typedef VkImage GfxImage;
typedef VkImageView GfxImageView;
typedef VulkanSamplerFilter GfxSamplerFilter;
typedef VkPipeline GfxPipeline;
typedef VkPipelineLayout GfxPipelineLayout;

struct GfxSubmitInfo;

GfxCommandBuffer GfxCreateCommandBuffer(GfxCommandPool commandPool);
void GfxSubmitCommandBuffers(GfxCommandQueueType queueType, GfxSubmitInfo &submitInfo, GfxFence fence);
void GfxFreeCommandBuffers(GfxCommandPool pool, s32 count, GfxCommandBuffer *buffers);
void GfxEndCommandBuffer(GfxCommandBuffer buffer);

GfxCommandPool GfxCreateCommandPool(GfxCommandQueueType queueType);
void GfxResetCommandPool(GfxCommandPool pool);

GfxBuffer GfxCreateBuffer(u32 size, GfxBufferUsageFlags usage);
void GfxDestroyBuffer(GfxBuffer buffer);
void GfxRecordCopyBufferCommand(GfxCommandBuffer buffer, u32 size, GfxBuffer source, GfxBuffer destination, u32 sourceOffset, u32 destinationOffset);
GfxMemoryRequirements GfxGetBufferMemoryRequirements(GfxBuffer buffer);
void GfxBindBufferMemory(GfxBuffer buffer, GfxMemory memory, u32 memoryOffset);

bool GfxAllocateMemory(u32 size, GfxMemoryType memoryType, GfxMemory *memory);
void *GfxMapMemory(GfxMemory memory, u32 size, u32 offset);

GfxShaderModule GfxCreateShaderModule(GfxShaderStage stage, const String &spirv);

GfxFence GfxCreateFence(bool startSignalled);
bool GfxWasFenceSignalled(GfxFence fence);
void GfxWaitForFences(u32 count, GfxFence *fences, bool waitForAllFences, u64 timeout);
void GfxResetFences(u32 count, GfxFence *fences);

GfxSwapchain GfxCreateSwapchain();
u32 GfxAcquireNextSwapchainImage(GfxSwapchain swapchain, u32 currentFrameIndex);
u32 GfxGetSwapchainImageCount(GfxSwapchain swapchain);
void GfxGetSwapchainImageViews(GfxSwapchain swapchain, u32 count, GfxImageView *imageViews);
void GfxPresentSwapchainImage(GfxSwapchain swapchain, u32 swapchainImageIndex, u32 currentFrame);

GfxDescriptorPool GfxCreateDescriptorPool(u32 swapchainImageCount);

GfxFramebuffer GfxCreateFramebuffer(GfxRenderPass renderPass, u32 width, u32 height, u32 attachmentCount, GfxImageView *attachments);

GfxMemoryRequirements GfxGetImageMemoryRequirements(GfxImage image);
GfxImage GfxCreateImage(u32 width, u32 height, GfxFormat format, GfxImageLayout initialLayout, GfxImageUsage usage, VkSampleCountFlagBits sampleCount);
void GfxBindImageMemory(GfxImage image, GfxMemory memory, u32 offset);
GfxImageView GfxCreateImageView(GfxImage image, GfxFormat format, GfxImageUsage usage);
void GfxTransitionImageLayout(GfxCommandBuffer commandBuffer, GfxImage image, GfxFormat format, GfxImageLayout oldLayout, GfxImageLayout newLayout);
void GfxRecordCopyBufferToImageCommand(GfxCommandBuffer commandBuffer, GfxBuffer buffer, GfxImage image, u32 imageWidth, u32 imageHeight);

#define GFX_ERROR_OBJECT 0

#define GFX_DESCRIPTOR_SET_COUNT 1

typedef struct Render_API_Context {
	VkPhysicalDevice physical_device;
	VkDevice device;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debug_messenger;
	VkSurfaceKHR window_surface;
	VkSurfaceFormatKHR window_surface_format;
	u32 graphics_queue_family;
	u32 present_queue_family;
	VkQueue graphics_queue;
	VkQueue present_queue;
	VkPresentModeKHR present_mode;
	VkDescriptorSetLayout descriptor_set_layouts[GFX_DESCRIPTOR_SET_COUNT];

	//u32 descriptor_set_count;
	//Vulkan_Descriptor_Set_Layouts descriptor_set_layouts;
	//Vulkan_Descriptor_Sets descriptor_sets;
	//VkSwapchainKHR swapchain;

	VkDeviceSize minimum_uniform_buffer_offset_alignment; // Any uniform or dynamic uniform buffer's offset inside a Vulkan memory block must be a multiple of this byte count.
	VkDeviceSize maximum_uniform_buffer_size; // Maximum size of any uniform buffer (including dynamic uniform buffers). @TODO: Move to sizes struct?
	VkSemaphore image_available_semaphores[GFX_MAX_FRAMES_IN_FLIGHT];
	VkSemaphore render_finished_semaphores[GFX_MAX_FRAMES_IN_FLIGHT];
} Render_API_Context;

#else

constexpr bool usingVulkanAPI = false;

#endif
