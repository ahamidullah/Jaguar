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
typedef u32 Texture_ID;

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

// @TODO: False sharing?
typedef struct GPU_Thread_Local_Context {
	VkCommandPool command_pools[VULKAN_MAX_FRAMES_IN_FLIGHT];
} GPU_Thread_Local_Context;

typedef struct GPU_Context {
	VkDevice device;
	GPU_Thread_Local_Context *thread_local;
} GPU_Context;

//Texture_ID GPU_Upload_Image(u8 *pixels, s32 image_width, s32 image_height);
//GPU_Mesh GPU_Upload_Mesh(u32 vertex_count, u32 sizeof_vertex, void *vertices, u32 index_count, u32 *indices);
