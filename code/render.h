#define MAX_FRAMES_IN_FLIGHT 2
#define MAX_DEBUG_RENDER_OBJECTS 500
#define MAX_ENTITY_MESHES 1000
#define MAX_LOADED_ASSET_COUNT 1000

typedef enum {
	LINE_PRIMITIVE,
} Render_Primitive;

typedef struct {
	u32 vertex_offset;
	u32 first_index;
	u32 index_count;
	V4 color;
	Render_Primitive render_primitive;
} Debug_Render_Object;

typedef struct Render_Memory_Allocation {
	u32 offset;
	u32 size;
	GPU_Memory memory;
	void *mapped_pointer; // Will be NULL if the render backend memory of the memory block is not host visible.
} Render_Memory_Allocation;

// A block allocator stores its memory in a linked list of fixed size blocks.
// Memory can then be suballocated out of each block, and when space in a block runs out, a new block is allocated from the render backend and added to the end of the linked list.
// Block allocators are dynamic and not thread-safe. All operations on a block allocator must lock the block allocator's mutex.

typedef struct Render_Memory_Block Render_Memory_Block;

typedef struct Render_Memory_Block {
	GPU_Memory memory;
	u32 frontier;
	u32 active_allocation_count;
	Render_Memory_Allocation active_allocations[VULKAN_MAX_MEMORY_ALLOCATIONS_PER_BLOCK];
	Render_Memory_Block *next;
} Render_Memory_Block;

typedef struct Render_Memory_Block_Allocator {
	u32 block_size;
	Render_Memory_Block *base_block;
	Render_Memory_Block *active_block;
	GPU_Memory_Type memory_type;
	Platform_Mutex mutex;
} Render_Memory_Block_Allocator;

typedef struct Render_Ring_Buffer_Allocator {
	GPU_Memory memory;
	u32 size;
	u32 read_offset;
	u32 write_offset;
} Render_Ring_Buffer_Allocator;

typedef struct Render_Memory {
	Render_Memory_Block_Allocator device_block_allocator;
	Render_Ring_Buffer_Allocator host_ring_buffer_allocator;
} Render_Memory;

// @TODO: Render_Mesh
typedef struct GPU_Mesh {
	Render_Memory_Allocation *memory;
	u32 indices_offset;
} GPU_Mesh;

typedef struct Render_Pass_Transient_Attachment_Creation_Parameters {
	u32 width;
	u32 height;
	GPU_Format format;
	u32 samples;
	bool depth;
} Render_Pass_Transient_Attachment_Creation_Parameters;

typedef enum Render_Pass_Attachment_Usage {
	RENDER_ATTACHMENT_READ_ONLY,
	RENDER_ATTACHMENT_WRITE_ONLY,
	RENDER_ATTACHMENT_READ_WRITE,
} Render_Pass_Attachment_Usage;

typedef struct Render_Pass_Attachment_Reference {
	u32 id;
	Render_Pass_Attachment_Usage usage;
} Render_Pass_Attachment_Reference;

typedef struct Render_Pass_Description {
	u32 transient_attachment_creation_count;
	Render_Pass_Transient_Attachment_Creation_Parameters *transient_attachment_creation_parameters;
	u32 color_attachment_count;
	Render_Pass_Attachment_Reference *color_attachments;
	Render_Pass_Attachment_Reference *depth_attachment;
} Render_Pass_Description;

typedef struct Render_Graph_External_Attachment {
	u32 id;
	GPU_Image image;
} Render_Graph_External_Attachment;

typedef struct Render_Graph_Description {
	u32 external_attachment_count;
	Render_Graph_External_Attachment *external_attachments;
	u32 render_pass_count;
	Render_Pass_Description *render_pass_descriptions;
} Render_Graph_Description;

typedef struct GPU_Context {
	union {
		Vulkan_Context vulkan;
	};
} GPU_Context;

// @TODO: False sharing?
typedef struct Render_Thread_Local_Context {
	GPU_Command_List_Pool command_list_pools[MAX_FRAMES_IN_FLIGHT];
} Render_Thread_Local_Context;

typedef struct Render_Context {
	M4 scene_projection;
	f32 focal_length; // The distance between the camera position and the near render plane in world space.
	f32 aspect_ratio; // Calculated from the render area dimensions, not the window dimensions.
	u32 debug_render_object_count;
	Debug_Render_Object debug_render_objects[MAX_DEBUG_RENDER_OBJECTS];
	Render_Memory memory;
	u32 current_frame_index;
	GPU_Context gpu_context;
	Render_Thread_Local_Context *thread_local;
} Render_Context;
