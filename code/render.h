#define MAX_DEBUG_RENDER_OBJECTS 500
#define MAX_ENTITY_MESHES 1000
#define MAX_LOADED_ASSET_COUNT 1000

typedef u32 Texture_ID;

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

typedef struct GPU_Memory_Allocators {
	GPU_Memory_Block_Allocator device_block_buffer;
	GPU_Memory_Block_Allocator device_block_image;
	GPU_Memory_Ring_Allocator staging_ring;
	GPU_Memory_Block_Allocator staging_block;
} GPU_Memory_Allocators;

typedef struct GPU_Image_Creation_Parameters {
	u32 width;
	u32 height;
	GPU_Format format;
	GPU_Image_Layout initial_layout;
	GPU_Image_Usage_Flags usage_flags;
	GPU_Sample_Count_Flags sample_count_flags;
} GPU_Image_Creation_Parameters;

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

typedef enum Render_API_ID {
	VULKAN_RENDER_API,
} Render_API_ID;

typedef struct Thread_Local_Render_Context {
	GPU_Command_Pool *command_pools;
	GPU_Command_Pool upload_command_pool; // @TODO: Double buffer and wait for one pool to empty to free it.
} Thread_Local_Render_Context;

typedef struct Render_Context {
	s32 render_thread_count;
	Thread_Local_Render_Context *thread_local_contexts;
	Render_API_Context api_context;
	//M4 scene_projection_matrix;
	//f32 focal_length; // The distance between the camera position and the near render plane in world space.
	f32 aspect_ratio; // Calculated from the render area dimensions, not the window dimensions.
	u32 debug_render_object_count;
	Debug_Render_Object debug_render_objects[MAX_DEBUG_RENDER_OBJECTS];
	u32 current_frame_index;
	GPU_Memory_Allocators gpu_memory_allocators;
	GPU_Swapchain swapchain;
	u32 swapchain_image_count;
	GPU_Framebuffer *framebuffers;
	GPU_Shader shaders[GPU_SHADER_COUNT];
	GPU_Descriptor_Pool descriptor_pool;
	GPU_Pipeline pipelines[GPU_SHADER_COUNT];
	GPU_Command_Pool asset_upload_command_pool;
	GPU_Shader_Descriptor_Sets rrdescriptor_sets;
	u32 descriptor_set_count;
	VkDescriptorSet descriptor_sets[100]; // @TODO
	struct {
		GPU_Render_Pass scene;
	} render_passes;

	u32 currentFrame;
	u32 nextFrame;
	GPU_Fence inFlightFences[GPU_MAX_FRAMES_IN_FLIGHT];
} Render_Context;
