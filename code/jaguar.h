void debug_print(const char *format, ...);

typedef enum {
	STANDARD_LOG,
	MINOR_ERROR_LOG,
	MAJOR_ERROR_LOG,
	CRITICAL_ERROR_LOG,
} Log_Type;

#define SCANCODE_COUNT 256
#define MOUSE_BUTTON_COUNT 3

void log_print_actual(Log_Type , const char *file, int line, const char *func, const char *format, ...);
void _abort_actual(const char *file, int line, const char *func, const char *fmt, ...);

typedef struct {
	char *data;
	u32 length;
	u32 capacity;
	u8 is_constant;
} String;

typedef struct {
	char *contents;
	u8 error;
} Read_File_Result;

typedef struct {
	u32 texture_id;
	u32 width;
	u32 height;
	s32 x_bearing;
	s32 y_bearing;
	s64 advance;
} Glyph_Info;

typedef enum GPU_Upload_Flags {
	GPU_UPLOAD_AS_SOON_AS_POSSIBLE,
} GPU_Upload_Flags;

/*
typedef struct Font {
	Glyph_Info glyph_info[256];

	s32 height;
	s32 ascender;
	s32 descender;
};
*/

////////////////////////////////////////
//
// Math.
//

#define print_f32(f) print_f32_actual(#f, (f))
#define print_v2(v2) print_v2_actual(#v2, (v2))
#define print_v3(v3) print_v3_actual(#v3, (v3))
#define print_v4(v4) print_v4_actual(#v4, (v4))
#define print_m4(m4) print_m4_actual(#m4, (m4))

u32 u32_max(u32 a, u32 b) {
	return a > b ? a : b;
}

u32 u32_min(u32 a, u32 b) {
	return a < b ? a : b;
}

typedef struct {
	f32 x, y;
} V2;

typedef struct {
	s32 x, y;
} V2s;

typedef struct {
	u32 x, y;
} V2u;

typedef struct {
	f32 x, y, z;
} V3;

typedef struct {
	f32 x, y, z, w;
} V4;

typedef struct {
	f32 m[3][3];
} M3;

typedef struct {
	f32 m[4][4];
} M4;

typedef struct {
	/*
	Quaternion() {}
	Quaternion(f32 x, f32 y, f32 z, f32 w) : x{x}, y{y}, z{z}, w{w} {}
	Quaternion(V3 v) : x{v.x}, y{v.y}, z{v.z}, w{0.0f} {}
	Quaternion(V3 axis, f32 angle) : im{sin(angle/2.0f)*axis}, w{cos(angle/2.0f)} {}
	*/
	f32 x, y, z, w;
} Quaternion;

////////////////////////////////////////
//
// Memory
//

typedef struct Chunk_Header {
	u8 *base_block;
	u8 *block_frontier;
	struct Chunk_Header *next_chunk;
} Chunk_Header;

typedef struct Block_Header {
	size_t byte_capacity;
	size_t bytes_used;
	struct Block_Header *next_block;
	struct Block_Header *previous_block;
} Block_Header;

typedef struct Free_Entry {
	size_t size;
	struct Free_Entry *next;
} Free_Entry;

typedef struct {
	size_t size;
	char *next;
	char *prev;
} Entry_Header;

Block_Header *create_memory_block();

typedef struct {
	Free_Entry *entry_free_head;
	char *last_entry;
	Block_Header *base_block;
	Block_Header *active_block;
} Memory_Arena;

typedef struct {
	char *data;
	u64 length; // @TODO: This really should be a File_Offset...
} String_Result;

//@TODO: Change lookup to a hash table.
//@TODO: Change lookup to a hash table.
//@TODO: Change lookup to a hash table.
//@TODO: Change lookup to a hash table.

typedef struct {
	V3 translation;
	Quaternion rotation;
} Transform;

////////////////////////////////////////
//
// Renderer
//

#define MAX_DEBUG_RENDER_OBJECTS 500
#define MAX_ENTITY_MESHES 1000
#define MAX_LOADED_ASSET_COUNT 1000

typedef enum {
	LINE_PRIMITIVE,
} Render_Primitive;

typedef struct {
	u32              vertex_offset;
	u32              first_index;
	u32              index_count;
	V4               color;
	Render_Primitive render_primitive;
} Debug_Render_Object;

typedef struct Render_Memory_Allocation {
	u32 offset;
	u32 size;
	GPU_Memory memory;
	void *mapped_pointer; // Will be NULL if the render backend memory of the memory block is not host visible.
} Render_Memory_Allocation;

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

typedef struct Render_Context {
	M4 scene_projection;
	f32 focal_length; // The distance between the camera position and the near render plane in world space.
	f32 aspect_ratio; // Calculated from the render area dimensions, not the window dimensions.

	u32 debug_render_object_count;
	Debug_Render_Object debug_render_objects[MAX_DEBUG_RENDER_OBJECTS];

	Render_Memory memory;
} Render_Context;

////////////////////////////////////////
//
// Material
//

typedef u32 Material_ID;
#define INVALID_ID ((u32)-1)

typedef enum {
	TEXTURED_STATIC_SHADER,
	UNTEXTURED_STATIC_SHADER,
	SHADOW_MAP_STATIC_SHADER,
	FLAT_COLOR_SHADER,

	SHADER_COUNT
} Shader_Type;

// @TODO @PREPROCCESSOR: Generate Material_IDs.

typedef struct Material {
	Shader_Type shader;
	union {
		struct {
			Texture_ID albedo_map;
			Texture_ID normal_map;
			Texture_ID roughness_map;
			Texture_ID metallic_map;
			Texture_ID ambient_occlusion_map;
		};
		struct {
			V4 flat_color;
		};
	};
} Material;

////////////////////////////////////////
//
// Mesh
//

typedef struct Vertex {
	V3 position;
	V3 color;
	V2 uv;
	V3 normal;
	V3 tangent;
} Vertex;

typedef struct Vertex1P {
	V3 position;
} Vertex1P;

typedef struct Bounding_Sphere {
	V3  center;
	f32 radius;
} Bounding_Sphere;

// @TODO: Lock asset?
typedef enum Asset_Load_Status {
	ASSET_UNLOADED,
	ASSET_LOADING,
	ASSET_LOADED,
} Asset_Load_Status;

typedef struct Mesh_Asset {
	// Hot. Potentially accessed every frame.
	Asset_Load_Status load_status;
	u32 submesh_count;
	Material *materials;
	u32 *submesh_index_counts;
	u32 vertex_offset;
	u32 first_index;
	GPU_Mesh gpu_mesh;

	// Cold. Accessed when the asset is loaded.
	u32 vertex_count;
	Vertex *vertices;
	u32 *indices;
	u32 index_count;
	Bounding_Sphere bounding_sphere;
} Mesh_Asset;

typedef struct Mesh_Instance {
	//Asset_Id asset_id;
	Transform transform;

	// Asset data. Only valid if the Mesh_Asset is loaded.
	Mesh_Asset *asset;
	/*
	GPU_Mesh gpu_mesh;
	Material *submesh_material_ids;
	u32 submesh_count;
	u32 vertex_offset;
	u32 first_index;
	u32 *submesh_index_counts;
	*/
} Mesh_Instance;

////////////////////////////////////////
//
// Animation
//

typedef struct Skeleton_Joint_Pose {
	Quaternion rotation;
	V3 translation;
	f32 scale;
} Skeleton_Joint_Pose;

typedef struct Skeleton_Joint_Skinning_Info {
	u32 vertex_influence_count;
	u32 *vertices;
	f32 *weights;
} Skeleton_Joint_Skinning_Info;

// @TODO: We could store animation transforms as 4x3 matrix, maybe?
typedef struct Skeleton_Asset {
	u8 joint_count;
	const char *joint_names; // @TODO: Get rid of joint names?
	Skeleton_Joint_Skinning_Info *joint_skinning_info;
	u8 *joint_parent_indices;
	M4 *joint_inverse_rest_pose;

	u8 leaf_node_count;
	u8 *leaf_node_parent_indices;
	M4 *leaf_node_translations; // @TODO @Memory: Could probably just be a V3 translation?
} Skeleton_Asset;

typedef struct Skeleton_Instance {
	Skeleton_Asset *asset;

	u32 local_joint_pose_count;
	Skeleton_Joint_Pose *local_joint_poses; // Currently, this is parent * node_transform, but it should probably just be node_transform for blending purposes.

	u32 global_joint_pose_count;
	M4 *global_joint_poses; // Currently, this is parent * node_transform, but it should probably just be node_transform for blending purposes.
} Skeleton_Instance;

typedef struct Animation_Sample {
	u32 joint_pose_count;
	Skeleton_Joint_Pose *joint_poses;
} Animation_Sample;

typedef struct Animation_Asset {
	Skeleton_Asset *skeleton;

	u32 sample_count;
	Animation_Sample *samples;

	f32 frame_count;
	f32 frames_per_second;
	s8 looped;
} Animation_Asset;

typedef struct Animation_Instance {
	Animation_Asset *asset;
	f32 time;
	u32 current_frame;
} Animation_Instance;

////////////////////////////////////////
//
// Game Data
//

typedef struct Camera {
	M4 view_matrix;

	V3 position;
	V3 forward;
	V3 side;
	V3 up;

	f32 field_of_view;
	f32 yaw;
	f32 pitch;
	f32 speed;
} Camera;

typedef enum Asset_ID {
	GUY_ASSET,
	GUY2_ASSET,
	GUY3_ASSET,
	GUN_ASSET,
	ANVIL_ASSET,
	ASSET_COUNT
} Asset_ID;

typedef struct Ring_Buffer {
	volatile s32 read_index;
	volatile s32 write_index;
	volatile u32 read_count;
} Ring_Buffer;

typedef struct Meshes_Waiting_For_GPU_Upload {
	GPU_Fence elements[100]; // @TODO
	Mesh_Asset *assets[100];
	Ring_Buffer ring_buffer;
} Meshes_Waiting_For_GPU_Upload;

typedef struct Material_Asset {
	Asset_Load_Status load_status;
} Material_Asset;

typedef struct Material_GPU_Fences {
	volatile u32 count;
	GPU_Fence fences[10]; // @TODO
} Material_GPU_Fences;

typedef struct Materials_Waiting_For_GPU_Upload {
	Material_GPU_Fences elements[100]; // @TODO
	Material_Asset *assets[100];
	Ring_Buffer ring_buffer;
} Materials_Waiting_For_GPU_Upload;

typedef struct Assets_Waiting_For_GPU_Upload {
	Meshes_Waiting_For_GPU_Upload meshes;
	Materials_Waiting_For_GPU_Upload materials;
} Assets_Waiting_For_GPU_Upload;

typedef struct {
	void *lookup[ASSET_COUNT]; // @TODO: Use a hash table.
	Asset_Load_Status load_statuses[ASSET_COUNT];
	Assets_Waiting_For_GPU_Upload waiting_for_gpu_upload;
	//Material materials[MAX_MATERIALS];    // @TODO: This really should be the exact count of materials, we know how many there will be from the directory, just need to preprocess.
	//u32 material_count;
	//Texture *textures;
	//Mesh_Asset *meshes;
} Game_Assets;

struct Submesh_Instance {
	u32 vertex_offset;
	u32 first_index;
	u32 index_count;
};

typedef struct {
	// @TODO: Store elements grouped spatially (BVH?) so that, after frustum culling, visible meshes are more likely to be contigous in memory.
	u32 count;
	Mesh_Instance instances[MAX_ENTITY_MESHES];
	Bounding_Sphere bounding_spheres[MAX_ENTITY_MESHES];

	// Per-mesh data
	//Mesh_Render_Info render_info[MAX_ENITTY_MESHES];
	//Transform        transforms[MAX_ENITTY_MESHES];
	//Bounding_Sphere  bounding_spheres[MAX_ENITTY_MESHES];
	//u32              submesh_counts[MAX_ENITTY_MESHES];
	//u32              count;

	// Per-submesh data
	//u32      submesh_index_counts[MAX_ENITTY_SUBMESHES];
	//Material submesh_materials[MAX_ENITTY_SUBMESHES];
	//u32      submesh_count;
} Entity_Meshes;

// @TODO: Use sparse arrays to store entity attributes.
typedef struct {
	Transform transforms[100]; // @TODO
	u32 transform_count;

	Entity_Meshes meshes;

	u32 ids[100]; // @TODO
	u32 id_count;
} Game_Entities;

typedef struct Game_Frame_Context {
	u32 frame_number;
	Memory_Arena arena;
} Game_Frame_Context;

typedef struct Thread_Local_Jobs_Context {
} Thread_Local_Jobs_Context;

typedef struct Platform_Thread_Local_Context {
} Platform_Thread_Local_Context;

typedef struct Game_Jobs_Context {
	u32 worker_thread_count;
} Game_Jobs_Context;

typedef struct {
} Game_Render_Context;

typedef struct Platform_Context {
} Platform_Context;

typedef struct Thread_Local_Game_State {
	Thread_Local_Jobs_Context jobs_context;
	Platform_Thread_Local_Context platform_context;
	GPU_Thread_Local_Context gpu_context;
} Thread_Local_Game_State;

typedef struct Game_Thread_Memory_Heap {
} Game_Thread_Memory_Heap;

typedef struct Game_Main_Memory_Heap {
} Game_Main_Memory_Heap;

typedef struct Game_Memory {
	Game_Main_Memory_Heap main_heap;
	u32 thread_heap_count;
	Game_Thread_Memory_Heap *thread_heaps;
} Game_Memory;

typedef struct Game_State {
	Game_Execution_Status execution_status;
	Game_Input input;
	Game_Memory memory;
	Game_Assets assets;
	Game_Entities entities;
	Game_Jobs_Context jobs_context;
	Game_Frame_Context frame_context;
	Game_Render_Context render_context;
	Camera camera; // @TODO: Rename to Game_Camera.
	Platform_Context platform_context;
	GPU_Context gpu_context;

	//u32 thread_count;
	//Thread_Local_Game_State *thread_local;

	Memory_Arena frame_arena;
	Memory_Arena permanent_arena;
} Game_State;
