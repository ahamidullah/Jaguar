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

typedef enum Game_Execution_Status {
	GAME_RUNNING,
	GAME_PAUSED,
	GAME_EXITING,
} Game_Execution_Status;

typedef enum Log_Type {
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

#define print_v4(v4) print_v4_actual(#v4, v4)
#define print_v3(v3) print_v3_actual(#v3, v3)
#define print_v2(v2) print_v2_actual(#v2, v2)
#define print_m4(m4) print_m4_actual(#m4, m4)

typedef struct V2 {
	f32 x, y;
} V2;

typedef struct V2s {
	s32 x, y;
} V2s;

typedef struct V2u {
	u32 x, y;
} V2u;

typedef struct V3 {
	f32 x, y, z;
} V3;

typedef struct V4 {
	f32 x, y, z, w;
} V4;

typedef struct M3 {
	f32 m[3][3];
} M3;

typedef struct M4 {
	f32 m[4][4];
} M4;

typedef struct Quaternion {
	//Quaternion() {}
	//Quaternion(f32 x, f32 y, f32 z, f32 w) : x{x}, y{y}, z{z}, w{w} {}
	//Quaternion(V3 v) : x{v.x}, y{v.y}, z{v.z}, w{0.0f} {}
	//Quaternion(V3 axis, f32 angle) : im{sin(angle/2.0f)*axis}, w{cos(angle/2.0f)} {}
	f32 x, y, z, w;
} Quaternion;

typedef struct IO_Buttons {
	u8 *down;
	u8 *pressed;
	u8 *released;
} IO_Buttons;

typedef struct Mouse {
	s32 wheel;
	s32 x, y;
	s32 delta_x, delta_y;
	f32 raw_delta_x, raw_delta_y;
	f32 sensitivity;
	IO_Buttons buttons;
} Mouse;

typedef struct Game_Input {
	Mouse mouse;
	IO_Buttons keyboard;
} Game_Input;

typedef struct Camera {
	M4 view_matrix;

	V3 position;
	V3 forward;
	V3 side;
	V3 up;

	f32 yaw;
	f32 pitch;
	f32 speed;
} Camera;

typedef struct Read_File_Result {
	char *contents;
	u8 error;
} Read_File_Result;

typedef struct Glyph_Info {
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

typedef struct Entry_Header {
	size_t size;
	char *next;
	char *prev;
} Entry_Header;

Block_Header *create_memory_block();

typedef struct Memory_Arena {
	Free_Entry *entry_free_head;
	char *last_entry;
	Block_Header *base_block;
	Block_Header *active_block;
} Memory_Arena;

typedef struct String_Result {
	char *data;
	u64 length; // @TODO: This really should be a File_Offset...
} String_Result;

//@TODO: Change lookup to a hash table.
//@TODO: Change lookup to a hash table.
//@TODO: Change lookup to a hash table.
//@TODO: Change lookup to a hash table.

typedef u32 Material_ID;

#define MAX_MATERIAL_COUNT 100

typedef struct Vertex {
	V3 position;
	V3 color;
	V2 uv;
	V3 normal;
} Vertex;

/*
// @TODO: Move vertex and index data into the model asset.
typedef struct Mesh_Asset {
	Material_ID material_id;

	Vertex *vertices;
	u32 vertex_count;

	u32 *indices;
	u32 index_count;
} Mesh_Asset;
*/

typedef struct Model_Asset {
	Vertex *vertices;
	u32 vertex_count;

	u32 *indices;
	u32 index_count;

	u32 *mesh_index_counts;
	u32 mesh_count;

	u32 vertex_offset;
	u32 first_index;
} Model_Asset;

typedef struct {
	V3 translation;
	Quaternion rotation;
	V3 scale;
} Transform;

typedef struct {
	u32 *index_counts;
	u32 mesh_count;

	Transform transform;
	u32 vertex_offset;
	u32 first_index;
} Model_Instance;

/*
typedef struct Model_Instance {
	u32 mesh_count;
	u32 *index_counts;

	Transform transform;
	u32 vertex_offset;
	u32 first_index;

	//u32 vertex_count;
	//u32 uniform_offset;
	//Material_ID *material_ids;
} Model_Instance;
*/

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

typedef struct Animation_Instance {
	Animation_Asset *asset;
	f32 time;
	u32 current_frame;
} Animation_Instance;

typedef enum Asset_ID {
	GUY1_ASSET,
	GUY2_ASSET,
	GUY3_ASSET,
	GUN_ASSET,
	NANOSUIT_ASSET,
} Asset_ID;

typedef enum Shader_Type {
	TEXTURED_STATIC_SHADER,
	UNTEXTURED_STATIC_SHADER,
	SHADOW_MAP_STATIC_SHADER,

	SHADER_COUNT
} Shader_Type;

typedef u32 Texture_ID;

typedef struct Material {
	Shader_Type shader;

	V3 diffuse_color;
	V3 specular_color;

	Texture_ID diffuse_map;
	Texture_ID specular_map;
	Texture_ID normal_map;
} Material;

#define MAX_LOADED_ASSET_COUNT 1000

typedef struct {
	Material materials[MAX_MATERIAL_COUNT];
	s32 material_count;

	void *lookup[MAX_LOADED_ASSET_COUNT]; // @TODO: Use a hash table.
} Game_Assets;

// Hash table.
//typedef struct {
	//Transform_Component *transform;
	//Render_Component *render;
//} Game_Components;

#define MAX_MODEL_INSTANCES 1000

typedef struct Game_State {
	Game_Execution_Status execution_status;
	Game_Input input;
	Game_Assets assets;
	Camera camera;

	Model_Instance model_instances[MAX_MODEL_INSTANCES];
	u32 model_instance_count;

	Memory_Arena frame_arena;
	Memory_Arena permanent_arena;
} Game_State;
