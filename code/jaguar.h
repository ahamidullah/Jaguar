void debug_print(const char *format, ...);

typedef enum {
	STANDARD_LOG, // @TODO: Get rid of this in favor of INFO_LOG.
	INFO_LOG,
	ERROR_LOG,
	ABORT_LOG,
} Log_Type;

#define SCANCODE_COUNT 256
#define MOUSE_BUTTON_COUNT 3

void log_print_actual(Log_Type , const char *file, int line, const char *func, const char *format, ...);
void _abort_actual(const char *file, int line, const char *func, const char *fmt, ...);

#include "strings.h"

typedef struct Read_File_Result {
	char *data;
	s32 size;
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

#include "gpu.h"
#include "render.h"

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

	_SHADER_COUNT
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
	// @TODO: Seperate hot and cold data?
	// Hot. Potentially accessed every frame.
	Asset_Load_Status load_status; // @TODO: Keep seperate lists for loaded and unloaded assets?
	u32 submesh_count;
	Material *materials;
	u32 *submesh_index_counts;
	u32 vertex_offset;
	u32 first_index;
	GPU_Indexed_Geometry gpu_mesh;

	// Cold. Accessed when the asset is loaded.
	u32 vertex_count;
	//Vertex *vertices;
	//u32 *indices;
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
	M4 projection_matrix;

	V3 position;
	V3 forward;
	V3 side;
	V3 up;

	f32 field_of_view;
	f32 focal_length;
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

/*
typedef struct Ring_Buffer {
	volatile s32 read_index;
	volatile s32 write_index;
	volatile u32 read_count;
} Ring_Buffer;
*/

#define Ring_Buffer(size) \
	volatile s32 read_index; \
	volatile s32 write_index; \
	volatile u32 read_count; \
	bool ready[size];

typedef struct Meshes_Waiting_For_GPU_Upload {
	GPU_Fence elements[100]; // @TODO
	Mesh_Asset *assets[100];
	//Ring_Buffer ring_buffer;
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
	//Ring_Buffer ring_buffer;
} Materials_Waiting_For_GPU_Upload;

typedef struct Assets_Waiting_For_GPU_Upload {
	Meshes_Waiting_For_GPU_Upload meshes;
	Materials_Waiting_For_GPU_Upload materials;
} Assets_Waiting_For_GPU_Upload;

// @TODO: Move this to a seperate file?
typedef struct Double_Buffer {
	s32 inactive_buffer_index;
	s32 active_buffer_element_count;
	s32 inactive_buffer_element_count;
} Double_Buffer;

#define Create_Double_Buffer(container) \
	do { \
		container.write_buffer = &container.buffers[0]; \
		container.write_head = container.buffers[0].command_buffers ; \
		container.read_buffer_element_count = 0; \
		container.read_buffer = &container.buffers[1]; \
		container.write_buffer->ready_count = 0; \
		container.read_buffer->ready_count = 0; \
	} while (0)

#define Switch_Double_Buffer(container) \
	do { \
		if (container.write_buffer == &container.buffers[0]) { \
			container.write_buffer = &container.buffers[1]; \
			container.read_buffer = &container.buffers[0]; \
		} else { \
			container.write_buffer = &container.buffers[0]; \
			container.read_buffer = &container.buffers[1]; \
		} \
		typeof(container.write_head) old_write_head = container.write_head; \
		container.write_buffer->ready_count = 0; \
		container.write_head = container.write_buffer->command_buffers; \
		container.read_buffer_element_count = old_write_head - container.read_buffer->command_buffers; \
		/*Console_Print("read_buffer_element_count: %d %d owh: %x, w: %x, r: %x\n", container.read_buffer_element_count, container.read_buffer->ready_count, old_write_head, container.write_buffer->command_buffers, container.read_buffer->command_buffers);*/ \
		while (container.read_buffer_element_count != container.read_buffer->ready_count) { \
			/**/ \
		} \
	} while (0)

#define Atomic_Write_To_Double_Buffer(container, command_buffer, counter) \
	do { \
		GPU_Command_Buffer *write_pointer; \
		do { \
			write_pointer = container.write_head; \
			Platform_Compare_And_Swap_Pointers((void *volatile *)&container.write_head, write_pointer, write_pointer + 1); \
		} while (container.write_head != write_pointer + 1); \
		*write_pointer = command_buffer; \
		s32 buffer_index = (write_pointer < container.buffers[1].command_buffers) ? 0 : 1; \
		s32 element_index = write_pointer - container.buffers[buffer_index].command_buffers; \
		container.buffers[buffer_index].gpu_upload_counters[element_index] = counter; \
		container.buffers[buffer_index].ready_count += 1; \
		/*Console_Print("buffer_index: %d %d\n", buffer_index, element_index);*/ \
	} while (0)

#define MAX_GPU_UPLOAD_COMMAND_BUFFERS 128
#define MAX_GPU_UPLOAD_FENCES 128
#define MAX_COMMAND_BUFFERS_PER_GPU_UPLOAD_FENCE 32

typedef struct Asset_GPU_Upload_Counter {
	s32 gpu_command_buffer_count;
	Asset_Load_Status *load_status;
} Asset_GPU_Upload_Counter;

typedef struct Upload_Buffer {
	s32 ready_count;
	GPU_Command_Buffer command_buffers[MAX_GPU_UPLOAD_COMMAND_BUFFERS];
	Asset_GPU_Upload_Counter *gpu_upload_counters[MAX_GPU_UPLOAD_COMMAND_BUFFERS];
} Upload_Buffer;

typedef struct Game_Assets {
	void *lookup[ASSET_COUNT]; // @TODO: Use a hash table.
	struct {
		Upload_Buffer buffers[2];
		GPU_Command_Buffer *write_head;
		s32 read_buffer_element_count;
		Upload_Buffer *read_buffer;
		Upload_Buffer *write_buffer;
	} gpu_upload_command_buffers;
	s32 gpu_upload_fence_count;
	GPU_Fence gpu_upload_fences[MAX_GPU_UPLOAD_FENCES];
	s32 pending_gpu_upload_counter_counts[MAX_GPU_UPLOAD_FENCES];
	Asset_GPU_Upload_Counter *pending_gpu_upload_counters[MAX_GPU_UPLOAD_FENCES][MAX_COMMAND_BUFFERS_PER_GPU_UPLOAD_FENCE];
	GPU_Command_Pool gpu_upload_command_pool;
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

typedef struct Game_Jobs_Context {
	u32 worker_thread_count;
} Game_Jobs_Context;

typedef struct Platform_Context {
} Platform_Context;

typedef struct Thread_Memory_Heap {
} Thread_Memory_Heap;

typedef struct Main_Memory_Heap {
} Main_Memory_Heap;

typedef struct Game_Memory {
	Main_Memory_Heap main_heap;
	u32 thread_heap_count;
	Thread_Memory_Heap *thread_heaps;
} Game_Memory;

__thread u32 thread_index;

typedef struct Game_State {
	Game_Execution_Status execution_status;
	Game_Input input;
	Game_Memory memory;
	Game_Assets assets;
	Game_Entities entities;
	Game_Jobs_Context jobs_context;
	Render_Context render_context;
	Camera camera; // @TODO: Rename to Game_Camera.
	Platform_Context platform_context;

	Memory_Arena frame_arena;
	Memory_Arena permanent_arena;
} Game_State;
