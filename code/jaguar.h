///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Shared Declarations
//

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;
typedef float    f32;
typedef double   f64;

typedef enum {
	GAME_RUNNING,
	GAME_PAUSED,
	GAME_EXITING,
} Game_Execution_Status;

typedef enum {
	STANDARD_LOG,
	MINOR_ERROR_LOG,
	MAJOR_ERROR_LOG,
	CRITICAL_ERROR_LOG,
} Log_Type;

#define INVALID_CODE_PATH assert(!"Invalid code path.");

#define MILLISECONDS(t) (t * 1000)

#define KILOBYTE(b) ((size_t)b*1024)
#define MEGABYTE(b) (KILOBYTE(b)*1024)
#define GIGABYTE(b) (MEGABYTE(b)*1024)

#define ARRAY_COUNT(x) (sizeof(x)/sizeof(x[0]))

#define SCANCODE_COUNT 256
#define MOUSE_BUTTON_COUNT 3

#define _abort(fmt, ...) _abort_actual(__FILE__, __LINE__, __func__, fmt, ## __VA_ARGS__)
#define log_print(log_type, fmt, ...) log_print_actual(log_type, __FILE__, __LINE__, __func__, fmt, ## __VA_ARGS__)

void log_print_actual(Log_Type , const char *file, int line, const char *func, const char *format, ...);
void _abort_actual(const char *file, int line, const char *func, const char *fmt, ...);

typedef struct {
	char *data;
	u32   length;
} String;

////////////////////////////////////////
//
// Math
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
// Input
//

typedef struct {
	u8 *down;
	u8 *pressed;
	u8 *released;
} IO_Buttons;

typedef struct {
	s32 wheel;
	s32 x, y;
	s32 delta_x, delta_y;
	f32 raw_delta_x, raw_delta_y;
	f32 sensitivity;
	IO_Buttons buttons;
} Mouse;

typedef struct {
	Mouse mouse;
	IO_Buttons keyboard;
} Game_Input;

typedef struct {
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
// Material
//

typedef u32 Material_ID;
typedef u32 Texture_ID;
#define INVALID_ID ((u32)-1)

typedef enum {
	TEXTURED_STATIC_SHADER,
	UNTEXTURED_STATIC_SHADER,
	SHADOW_MAP_STATIC_SHADER,
	FLAT_COLOR_SHADER,

	SHADER_COUNT
} Shader_Type;

// @TODO @PREPROCCESSOR: Generate Material_IDs.

typedef struct {
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

typedef struct {
	V3 position;
	V3 color;
	V2 uv;
	V3 normal;
	V3 tangent;
} Vertex;

typedef struct {
	V3 position;
} Vertex1P;

typedef struct {
	V3  center;
	f32 radius;
} Bounding_Sphere;

typedef struct {
	Vertex *vertices;
	u32     vertex_count;

	u32 *indices;
	u32  index_count;

	Material *materials;
	u32      *submesh_index_counts;
	u32       submesh_count;

	u32 vertex_offset;
	u32 first_index;

	Bounding_Sphere bounding_sphere;
} Mesh_Asset;

typedef struct {
	Transform transform;
	Material *submesh_material_ids;
	u32 submesh_count;
	u32 vertex_offset;
	u32 first_index;
	u32 *submesh_index_counts;
} Mesh_Instance;

typedef struct {
	u32 vertex_offset;
	u32 first_index;
	u32 first_submesh_index;
} Mesh_Render_Info;

////////////////////////////////////////
//
// Animation
//

typedef struct {
	Quaternion rotation;
	V3 translation;
	f32 scale;
} Skeleton_Joint_Pose;

typedef struct {
	u32 vertex_influence_count;
	u32 *vertices;
	f32 *weights;
} Skeleton_Joint_Skinning_Info;

// @TODO: We could store animation transforms as 4x3 matrix, maybe?
typedef struct {
	u8 joint_count;
	const char *joint_names; // @TODO: Get rid of joint names?
	Skeleton_Joint_Skinning_Info *joint_skinning_info;
	u8 *joint_parent_indices;
	M4 *joint_inverse_rest_pose;

	u8 leaf_node_count;
	u8 *leaf_node_parent_indices;
	M4 *leaf_node_translations; // @TODO @Memory: Could probably just be a V3 translation?
} Skeleton_Asset;

typedef struct {
	Skeleton_Asset *asset;

	u32 local_joint_pose_count;
	Skeleton_Joint_Pose *local_joint_poses; // Currently, this is parent * node_transform, but it should probably just be node_transform for blending purposes.

	u32 global_joint_pose_count;
	M4 *global_joint_poses; // Currently, this is parent * node_transform, but it should probably just be node_transform for blending purposes.
} Skeleton_Instance;

typedef struct {
	u32 joint_pose_count;
	Skeleton_Joint_Pose *joint_poses;
} Animation_Sample;

typedef struct {
	Skeleton_Asset *skeleton;

	u32 sample_count;
	Animation_Sample *samples;

	f32 frame_count;
	f32 frames_per_second;
	s8 looped;
} Animation_Asset;

typedef struct {
	Animation_Asset *asset;
	f32 time;
	u32 current_frame;
} Animation_Instance;

////////////////////////////////////////
//
// Game Data
//

#define MAX_DEBUG_RENDER_OBJECTS  500
#define MAX_ENTITY_MESHES         1000
#define MAX_LOADED_ASSET_COUNT    1000

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

typedef struct {
	M4  scene_projection;
	f32 focal_length; // The distance between the camera position and the near render plane in world space.
	f32 aspect_ratio; // Calculated from the render area dimensions, not the window dimensions.

	Debug_Render_Object debug_render_objects[MAX_DEBUG_RENDER_OBJECTS];
	u32                 debug_render_object_count;
} Render_Context;

typedef enum {
	GUY_ASSET,
	GUY2_ASSET,
	GUY3_ASSET,
	GUN_ASSET,
	ANVIL_ASSET,
} Asset_ID;

typedef struct {
	void *lookup[MAX_LOADED_ASSET_COUNT]; // @TODO: Use a hash table.
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
	u32              count;
	Mesh_Instance    instances[MAX_ENTITY_MESHES];
	Bounding_Sphere  bounding_spheres[MAX_ENTITY_MESHES];
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
	u32       transform_count;

	Entity_Meshes meshes;

	u32 ids[100]; // @TODO
	u32 id_count;
} Game_Entities;

typedef struct {
	Game_Execution_Status execution_status;
	Game_Input input;
	Game_Assets assets;
	Game_Entities entities;

	Camera camera;

	Memory_Arena frame_arena;
	Memory_Arena permanent_arena;
} Game_State;
