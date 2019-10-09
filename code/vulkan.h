#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_xlib.h> 

#define VULKAN_MAX_FRAMES_IN_FLIGHT 2
#define VULKAN_MAX_MEMORY_ALLOCATIONS_PER_BLOCK 512

typedef VkFence GPU_Fence;
typedef VkCommandBuffer GPU_Command_List;
typedef VkCommandPool GPU_Command_List_Pool;
typedef VkDeviceMemory GPU_Memory;
typedef VkBuffer GPU_Buffer;
typedef VkSampler GPU_Sampler;
typedef u32 Texture_ID;
typedef VkBlendFactor GPU_Blend_Factor;
typedef VkBlendOp GPU_Blend_Operation;
typedef VkColorComponentFlags GPU_Color_Component_Flags;
typedef VkVertexInputRate GPU_Vertex_Input_Rate;
typedef VkFormat GPU_Format;
typedef VkShaderStageFlags GPU_Shader_Stage_Flags;
typedef VkShaderModule GPU_Shader_Module;
typedef VkRenderPass GPU_Render_Pass;
typedef VkDescriptorPool GPU_Descriptor_Pool;
typedef VkDescriptorType GPU_Descriptor_Type;
typedef VkPrimitiveTopology GPU_Pipeline_Topology;
typedef VkCompareOp GPU_Compare_Operation;
typedef VkDynamicState GPU_Dynamic_Pipeline_State;
typedef VkFramebuffer GPU_Framebuffer;
typedef VkSampleCountFlags GPU_Sample_Count;
typedef VkImageLayout GPU_Image_Layout;
typedef VkImageUsageFlags GPU_Image_Usage_Flags;
typedef VkSamplerAddressMode GPU_Sampler_Address_Mode;
typedef VkBorderColor GPU_Border_Color;

typedef enum GPU_Buffer_Usage_Flags {
	GPU_TRANSFER_DESTINATION_BUFFER = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	GPU_TRANSFER_SOURCE_BUFFER = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	GPU_VERTEX_BUFFER = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	GPU_INDEX_BUFFER = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
	GPU_UNIFORM_BUFFER = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
} GPU_Buffer_Usage_Flags;

typedef enum GPU_Memory_Type {
	GPU_DEVICE_MEMORY = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	GPU_HOST_MEMORY = (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT),
} GPU_Memory_Type;

typedef struct GPU_Image {
	VkImage image;
	VkImageView view;
	VkFormat format;
	VkImageLayout layout;
} GPU_Image;

typedef struct GPU_Sampler_Filter {
	VkFilter min;
	VkFilter mag;
	VkSamplerMipmapMode mipmap;
} GPU_Sampler_Filter;

typedef struct GPU_Descriptor_Set {
	VkDescriptorSet descriptor_set;
	VkDescriptorSetLayout layout;
} GPU_Descriptor_Set;

typedef struct GPU_Pipeline {
	VkPipelineLayout layout;
	VkPipeline pipeline;
} GPU_Pipeline;

typedef struct Vulkan_Context {
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
	VkSwapchainKHR swapchain;
	VkDeviceSize minimum_uniform_buffer_offset_alignment; // Any uniform or dynamic uniform buffer's offset inside a Vulkan memory block must be a multiple of this byte count.
	VkDeviceSize maximum_uniform_buffer_size; // Maximum size of any uniform buffer (including dynamic uniform buffers). @TODO: Move to sizes struct?
} Vulkan_Context;

//Texture_ID GPU_Upload_Image(u8 *pixels, s32 image_width, s32 image_height);
//GPU_Mesh GPU_Upload_Mesh(u32 vertex_count, u32 sizeof_vertex, void *vertices, u32 index_count, u32 *indices);
