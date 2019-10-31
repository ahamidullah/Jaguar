#if 0
typedef enum GPU_Color_Component_Flags {
	GPU_COLOR_COMPONENT_RED = 0x1,
	GPU_COLOR_COMPONENT_GREEN = 0x2,
	GPU_COLOR_COMPONENT_BLUE = 0x4,
	GPU_COLOR_COMPONENT_ALPHA = 0x8,
} GPU_Color_Component_Flags;

typedef enum GPU_Image_Usage_Flags {
	GPU_IMAGE_USAGE_TRANSFER_SRC = 0x00000001,
	GPU_IMAGE_USAGE_TRANSFER_DST = 0x00000002,
	GPU_IMAGE_USAGE_SAMPLED = 0x00000004,
	GPU_IMAGE_USAGE_STORAGE = 0x00000008,
	GPU_IMAGE_USAGE_COLOR_ATTACHMENT = 0x00000010,
	GPU_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT = 0x00000020,
	GPU_IMAGE_USAGE_TRANSIENT_ATTACHMENT = 0x00000040,
	GPU_IMAGE_USAGE_INPUT_ATTACHMENT = 0x00000080,
	GPU_IMAGE_USAGE_SHADING_RATE_IMAGE_NV = 0x00000100,
	GPU_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_EXT = 0x00000200,
} GPU_Image_Usage_Flags;

typedef enum GPU_Shader_Stage_Flags {
	GPU_SHADER_STAGE_VERTEX = 0x1,
	GPU_SHADER_STAGE_FRAGMENT = 0x2,
} GPU_Shader_Stage_Flags;

typedef enum GPU_Sample_Count_Flags {
	GPU_SAMPLE_COUNT_1 = 0x1,
	GPU_SAMPLE_COUNT_2 = 0x2,
	GPU_SAMPLE_COUNT_4 = 0x4,
	GPU_SAMPLE_COUNT_8 = 0x8,
	GPU_SAMPLE_COUNT_16 = 0x10,
	GPU_SAMPLE_COUNT_32 = 0x20,
	GPU_SAMPLE_COUNT_64 = 0x40,
} GPU_Sample_Count_Flags;

typedef enum GPU_Blend_Factor {
	GPU_BLEND_FACTOR_SRC_ALPHA,
	GPU_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
	GPU_BLEND_FACTOR_ONE,
	GPU_BLEND_FACTOR_ZERO,
} GPU_Blend_Factor;

typedef enum GPU_Blend_Operation {
	GPU_BLEND_OP_ADD,
} GPU_Blend_Operation;

typedef enum GPU_Vertex_Input_Rate {
	GPU_VERTEX_INPUT_RATE_VERTEX,
	GPU_VERTEX_INPUT_RATE_INSTANCE,
} GPU_Vertex_Input_Rate;

typedef enum GPU_Format {
	GPU_FORMAT_R32G32B32_SFLOAT,
	GPU_FORMAT_R32_UINT,
	GPU_FORMAT_D32_SFLOAT_S8_UINT,
	GPU_FORMAT_R8G8B8A8_UNORM,
	GPU_FORMAT_D16_UNORM,
	GPU_FORMAT_UNDEFINED,
} GPU_Format;

typedef enum GPU_Descriptor_Type {
	GPU_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	GPU_DESCRIPTOR_TYPE_DYNAMIC_UNIFORM_BUFFER,
	GPU_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	GPU_DESCRIPTOR_TYPE_SAMPLER,
	GPU_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
} GPU_Descriptor_Type;

typedef enum GPU_Pipeline_Topology {
	GPU_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	GPU_PRIMITIVE_TOPOLOGY_LINE_LIST,
} GPU_Pipeline_Topology;

typedef enum GPU_Compare_Operation {
	GPU_COMPARE_OP_NEVER,
	GPU_COMPARE_OP_LESS,
	GPU_COMPARE_OP_EQUAL,
	GPU_COMPARE_OP_LESS_OR_EQUAL,
	GPU_COMPARE_OP_GREATER,
	GPU_COMPARE_OP_NOT_EQUAL,
	GPU_COMPARE_OP_GREATER_OR_EQUAL,
	GPU_COMPARE_OP_ALWAYS,
} GPU_Compare_Operation;

typedef enum GPU_Dynamic_Pipeline_State {
	GPU_DYNAMIC_PIPELINE_STATE_VIEWPORT,
	GPU_DYNAMIC_PIPELINE_STATE_SCISSOR,
	GPU_DYNAMIC_PIPELINE_STATE_LINE_WIDTH,
	GPU_DYNAMIC_PIPELINE_STATE_DEPTH_BIAS,
	GPU_DYNAMIC_PIPELINE_STATE_BLEND_CONSTANTS,
	GPU_DYNAMIC_PIPELINE_STATE_DEPTH_BOUNDS,
	GPU_DYNAMIC_PIPELINE_STATE_STENCIL_COMPARE_MASK,
	GPU_DYNAMIC_PIPELINE_STATE_STENCIL_WRITE_MASK,
	GPU_DYNAMIC_PIPELINE_STATE_STENCIL_REFERENCE,
} GPU_Dynamic_Pipeline_State;

typedef enum GPU_Image_Layout {
	GPU_IMAGE_LAYOUT_UNDEFINED,
	GPU_IMAGE_LAYOUT_GENERAL,
	GPU_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	GPU_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	GPU_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
	GPU_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	GPU_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	GPU_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	GPU_IMAGE_LAYOUT_PREINITIALIZED,
	GPU_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
	GPU_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
	GPU_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	GPU_IMAGE_LAYOUT_SHARED_PRESENT_KHR,
	GPU_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV,
	GPU_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT,
	GPU_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR,
	GPU_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR,
} GPU_Image_Layout;

typedef enum GPU_Sampler_Address_Mode {
	GPU_SAMPLER_ADDRESS_MODE_REPEAT,
	GPU_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
	GPU_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
	GPU_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
	GPU_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE,
	GPU_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE_KHR,
} GPU_Sampler_Address_Mode;

typedef enum GPU_Border_Color {
	GPU_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
	GPU_BORDER_COLOR_INT_TRANSPARENT_BLACK,
	GPU_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
	GPU_BORDER_COLOR_INT_OPAQUE_BLACK,
	GPU_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
	GPU_BORDER_COLOR_INT_OPAQUE_WHITE,
} GPU_Border_Color;

typedef enum GPU_Buffer_Usage_Flags {
	GPU_TRANSFER_DESTINATION_BUFFER = 0x1,
	GPU_TRANSFER_SOURCE_BUFFER = 0x2,
	GPU_VERTEX_BUFFER = 0x4,
	GPU_INDEX_BUFFER = 0x8,
	GPU_UNIFORM_BUFFER = 0x10,
} GPU_Buffer_Usage_Flags;

typedef enum GPU_Command_Pool_Type {
	GPU_GRAPHICS_COMMAND_POOL,
} GPU_Command_Pool_Type;

typedef enum GPU_Memory_Type {
	GPU_DEVICE_MEMORY,// = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	GPU_HOST_MEMORY,// = (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT),
} GPU_Memory_Type;

typedef struct GPU_Fence {
	union {
		VkFence vulkan;
	};
} GPU_Fence;

typedef struct GPU_Command_Buffer {
	union {
		VkCommandBuffer vulkan;
	};
} GPU_Command_Buffer;

typedef struct GPU_Command_Pool {
	union {
		VkCommandPool vulkan;
	};
} GPU_Command_Pool;

typedef struct GPU_Memory {
	union {
		VkDeviceMemory vulkan;
	};
} GPU_Memory;

typedef struct GPU_Buffer {
	union {
		VkBuffer vulkan;
	};
} GPU_Buffer;

typedef struct GPU_Sampler {
	union {
		VkSampler vulkan;
	};
} GPU_Sampler;

typedef struct GPU_Shader_Module {
	union {
		VkShaderModule vulkan;
	};
} GPU_Shader_Module;

typedef struct GPU_Render_Pass {
	union {
		VkRenderPass vulkan;
	};
} GPU_Render_Pass;

typedef struct GPU_Descriptor_Pool {
	union {
		VkDescriptorPool vulkan;
	};
} GPU_Descriptor_Pool;

typedef struct GPU_Framebuffer {
	union {
		VkFramebuffer vulkan;
	};
} GPU_Framebuffer;

typedef struct GPU_Resource_Allocation_Requirements {
	u32 size;
	u32 alignment;
} GPU_Resource_Allocation_Requirements;

typedef struct GPU_Image {
	union {
		Vulkan_Image vulkan;
	};
} GPU_Image;

typedef struct GPU_Sampler_Filter {
	union {
		Vulkan_Sampler_Filter vulkan;
	};
} GPU_Sampler_Filter;

typedef struct GPU_Descriptor_Set {
	union {
		Vulkan_Descriptor_Set vulkan;
	};
} GPU_Descriptor_Set;

typedef struct GPU_Pipeline {
	union {
		Vulkan_Pipeline vulkan;
	};
} GPU_Pipeline;

typedef struct GPU_Swapchain {
	union {
		VkSwapchainKHR vulkan;
	};
} GPU_Swapchain;
#endif

typedef struct GPU_Memory_Allocation {
	GPU_Memory memory;
	u32 offset;
	void *mapped_pointer;
} GPU_Memory_Allocation;

typedef struct GPU_Subbuffer {
	GPU_Buffer buffer;
	u32 *offset;
} GPU_Subbuffer;

typedef struct GPU_Image_Allocation {
	GPU_Memory memory;
	u32 *offset;
} GPU_Image_Allocation;

// A block allocator stores its memory in a linked list of fixed size blocks.
// Memory can then be suballocated out of each block, and when space in a block runs out, a new block is allocated from the render backend and added to the end of the linked list.
// Block allocators are dynamic and not thread-safe. All operations on a block allocator must lock the block allocator's mutex.

typedef struct GPU_Buffer_Memory_Block GPU_Buffer_Memory_Block;
typedef struct GPU_Buffer_Memory_Block GPU_Buffer_Memory_Block;
typedef struct GPU_Image_Memory_Block GPU_Image_Memory_Block;

typedef struct GPU_Buffer_Memory_Block {
	GPU_Memory memory;
	GPU_Buffer buffer;
	u32 frontier; // @TODO
	void *mapped_pointer;
	u32 allocation_count;
	u32 allocations[VULKAN_MAX_MEMORY_ALLOCATIONS_PER_BLOCK];
	GPU_Buffer_Memory_Block *next;
} GPU_Buffer_Memory_Block;

#if 0
typedef struct GPU_Device_Buffer_Memory_Block {
	GPU_Memory memory;
	GPU_Buffer buffer;
	u32 frontier; // @TODO
	u32 allocation_count;
	u32 allocations[VULKAN_MAX_MEMORY_ALLOCATIONS_PER_BLOCK];
	GPU_Buffer_Memory_Block *next;
} GPU_Device_Buffer_Memory_Block;
#endif

typedef struct GPU_Image_Memory_Block {
	GPU_Memory memory;
	u32 frontier; // @TODO
	u32 allocation_count;
	u32 allocations[VULKAN_MAX_MEMORY_ALLOCATIONS_PER_BLOCK];
	GPU_Image_Memory_Block *next;
} GPU_Image_Memory_Block;

typedef struct GPU_Buffer_Block_Allocator {
	Platform_Mutex mutex;
	GPU_Memory_Type memory_type;
	u32 block_size;
	GPU_Buffer_Usage_Flags buffer_usage_flags;
	GPU_Buffer_Memory_Block *base_block;
	GPU_Buffer_Memory_Block *active_block;
} GPU_Buffer_Block_Allocator;

#if 0
typedef struct GPU_Device_Buffer_Block_Allocator {
	Platform_Mutex mutex;
	GPU_Memory_Type memory_type;
	u32 block_size;
	GPU_Buffer_Usage_Flags buffer_usage_flags;
	GPU_Device_Buffer_Memory_Block *base_block;
	GPU_Device_Buffer_Memory_Block *active_block;
} GPU_Device_Buffer_Block_Allocator;
#endif

typedef struct GPU_Image_Block_Allocator {
	Platform_Mutex mutex;
	GPU_Memory_Type memory_type;
	u32 block_size;
	GPU_Image_Memory_Block *base_block;
	GPU_Image_Memory_Block *active_block;
} GPU_Image_Block_Allocator;

typedef struct GPU_Buffer_Ring_Allocator {
	GPU_Memory memory;
	GPU_Memory_Type memory_type;
	GPU_Buffer buffer;
	void *mapped_pointer;
	s32 size;
	s32 read_index;
	s32 write_index;
} GPU_Buffer_Ring_Allocator;




typedef struct GPU_Memory_Block GPU_Memory_Block;

typedef struct GPU_Memory_Block {
	GPU_Memory memory;
	void *mapped_pointer;
	u32 frontier;
	u32 allocation_count;
	GPU_Memory_Allocation allocations[VULKAN_MAX_MEMORY_ALLOCATIONS_PER_BLOCK];
	GPU_Memory_Block *next;
} GPU_Memory_Block;

typedef struct GPU_Memory_Block_Allocator {
	Platform_Mutex mutex;
	u32 block_size;
	GPU_Memory_Block *base_block;
	GPU_Memory_Block *active_block;
	GPU_Memory_Type memory_type;
} GPU_Memory_Block_Allocator;

typedef struct GPU_Memory_Ring_Allocator {
	GPU_Memory memory;
	void *mapped_pointer;
	u32 size;
	u32 read_index;
	u32 write_index;
} GPU_Memory_Ring_Allocator;

typedef struct GPU_Indexed_Geometry {
	GPU_Buffer vertex_buffer;
	GPU_Buffer index_buffer;
} GPU_Indexed_Geometry;
