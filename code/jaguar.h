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

s32 window_width, window_height;

enum Game_Execution_Status {
	GAME_RUNNING,
	GAME_PAUSED,
	GAME_EXITING,
};

enum Log_Type {
	STANDARD_LOG,
	MINOR_ERROR_LOG,
	MAJOR_ERROR_LOG,
	CRITICAL_ERROR_LOG,
};

#define INVALID_CODE_PATH assert(!"Invalid code path.");

#define MILLISECONDS(t) (t * 1000)

#define KILOBYTE(b) ((size_t)b*1024)
#define MEGABYTE(b) (KILOBYTE(b)*1024)
#define GIGABYTE(b) (MEGABYTE(b)*1024)

#define ARRAY_COUNT(x) (sizeof(x)/sizeof(x[0]))

template <typename F>
struct Scope_Exit {
	Scope_Exit(F _f) : f(_f) {}
	~Scope_Exit() { f(); }
	F f;
};

template <typename F>
Scope_Exit<F>
make_scope_exit(F f)
{
	return Scope_Exit<F>(f);
}

#define DO_STRING_JOIN(arg1, arg2) arg1 ## arg2
#define STRING_JOIN(arg1, arg2) DO_STRING_JOIN(arg1, arg2)
#define DEFER(code) auto STRING_JOIN(scope_exit_, __LINE__) = make_scope_exit([=](){code;})

#define _abort(fmt, ...) _abort_actual(__FILE__, __LINE__, __func__, fmt, ## __VA_ARGS__)
#define log_print(log_type, fmt, ...) log_print_actual(log_type, __FILE__, __LINE__, __func__, fmt, ## __VA_ARGS__)

void log_print_actual(Log_Type , const char *file, int line, const char *func, const char *format, ...);
void _abort_actual(const char *file, int line, const char *func, const char *fmt, ...);

#define print_v4(v4) print_v4_actual(#v4, v4)
#define print_v3(v3) print_v3_actual(#v3, v3)
#define print_v2(v2) print_v2_actual(#v2, v2)
#define print_m4(m4) print_m4_actual(#m4, m4)

struct V2 {
	f32 x, y;
	f32 &operator[](int i);
};

struct V2s {
	s32 x, y;
};

struct V2u {
	u32 x, y;
};

struct V3 {
	f32 x, y, z;
	f32 operator[](int i) const;
	f32 &operator[](int i);
};

struct V4 {
	f32 x, y, z, w;
	f32 operator[](int i) const;
	f32 &operator[](int i);
};

struct M3 {
	V3 &operator[](s32 i);

	f32 m[3][3];
};

struct M4 {
	V4 &operator[](s32 i);

	f32 m[4][4];
};

struct Quaternion {
	//Quaternion() {}
	//Quaternion(f32 x, f32 y, f32 z, f32 w) : x{x}, y{y}, z{z}, w{w} {}
	//Quaternion(V3 v) : x{v.x}, y{v.y}, z{v.z}, w{0.0f} {}
	//Quaternion(V3 axis, f32 angle) : im{sin(angle/2.0f)*axis}, w{cos(angle/2.0f)} {}
	f32 x = 0.0f;
	f32 y = 0.0f;
	f32 z = 0.0f;
	f32 w = 1.0f;
};

template <u32 BUTTON_COUNT>
struct IO_Buttons {
	u8 down[BUTTON_COUNT];
	u8 pressed[BUTTON_COUNT];
	u8 released[BUTTON_COUNT];
};

#define MOUSE_BUTTON_COUNT 3
struct Mouse {
	s32 wheel;
	s32 x, y;
	s32 delta_x, delta_y;
	f32 raw_delta_x, raw_delta_y;
	//V2 position;
	//V2 delta_position;
	f32 sensitivity;
	IO_Buttons<MOUSE_BUTTON_COUNT> buttons;
};

#define MAX_SCANCODES 256
struct Game_Input {
	Mouse mouse;
	IO_Buttons<MAX_SCANCODES> keyboard;
};

struct Camera {
	M4 view_matrix;

	V3 position;
	V3 forward;
	V3 side;
	V3 up;

	f32 yaw;
	f32 pitch;
	f32 speed;
};

struct Read_File_Result {
	char *contents;
	u8 error;
};

struct Glyph_Info {
	u32 texture_id;
	u32 width;
	u32 height;
	s32 x_bearing;
	s32 y_bearing;
	s64 advance;
};

/*
struct Font {
	Glyph_Info glyph_info[256];

	s32 height;
	s32 ascender;
	s32 descender;
};
*/

template <typename T>
struct Array {
	s32 count;
	s32 capacity;
	T *data;

	T &operator[](s32 i);
};

template <typename T>
struct Static_Array {
	s32 capacity;
	T *data;

	T &operator[](s32 i);
};

struct Chunk_Header {
	u8 *base_block;
	u8 *block_frontier;
	Chunk_Header *next_chunk;
};

struct Block_Header {
	size_t byte_capacity;
	size_t bytes_used;
	Block_Header *next_block;
	Block_Header *previous_block;
};

struct Free_Entry {
	size_t size;
	Free_Entry *next;
};

struct Entry_Header {
	size_t size;
	char *next;
	char *prev;
};

Block_Header *create_memory_block();

struct Memory_Arena {
	Free_Entry *entry_free_head = NULL;
	char *last_entry = NULL;
	Block_Header *base_block = NULL;
	Block_Header *active_block = NULL;
};

struct String_Result {
	char *data;
	u64 length; // @TODO: This really should be a File_Offset...
};

//@TODO: Change lookup to a hash table.
//@TODO: Change lookup to a hash table.
//@TODO: Change lookup to a hash table.
//@TODO: Change lookup to a hash table.

template <typename T, typename U>
struct Asset_Info {
	void *memory = NULL;
	size_t size = 0;
	u32 instance_count = 0;
	U *instances = NULL;
	u32 *instance_lookup = NULL;
};

typedef u32 Material_ID;

#define MAX_MATERIAL_COUNT 100

struct Vertex {
	V3 position;
	V3 color;
	V2 uv;
	V3 normal;
};

struct Mesh_Asset {
/*
	u32 vao;
	u32 vbo;
	u32 ebo;
	u32 index_count;
	u32 texture_id;
*/
	//Material_Type material_type;
	Material_ID material_id;

	Vertex *vertices;
	u32 vertex_count;

	u32 *indices;
	u32 index_count;
};

struct Model_Asset {
	u32 mesh_count;
	Mesh_Asset *meshes;
	//std::vector<Mesh> meshes; // @TODO
};

struct Model_Instance {
	u32 mesh_count;
	Mesh_Asset *meshes;
};

struct Skeleton_Joint_Pose {
	Quaternion rotation;
	V3 translation;
	f32 scale;
};

struct Skeleton_Joint_Skinning_Info {
	u32 vertex_influence_count;
	u32 *vertices;
	f32 *weights;
};

// @TODO: We could store animation transforms as 4x3 matrix, maybe?
struct Skeleton_Asset {
	u8 joint_count;
	const char *joint_names; // @TODO: Get rid of this aiString.
	Skeleton_Joint_Skinning_Info *joint_skinning_info;
	u8 *joint_parent_indices;
	M4 *joint_inverse_rest_pose;

	u8 leaf_node_count;
	u8 *leaf_node_parent_indices;
	M4 *leaf_node_translations; // @TODO @Memory: Could probably just be a V3 translation?
};

struct Skeleton_Instance {
	Skeleton_Asset *asset;

	u32 local_joint_pose_count;
	Skeleton_Joint_Pose *local_joint_poses; // Currently, this is parent * node_transform, but it should probably just be node_transform for blending purposes.

	u32 global_joint_pose_count;
	M4 *global_joint_poses; // Currently, this is parent * node_transform, but it should probably just be node_transform for blending purposes.
};

struct Animation_Sample {
	u32 joint_pose_count;
	Skeleton_Joint_Pose *joint_poses;
};

struct Animation_Asset {
	Skeleton_Asset *skeleton;

	u32 sample_count;
	Animation_Sample *samples;

	f32 frame_count;
	f32 frames_per_second;
	s8 looped;
};

struct Animation_Instance {
	Animation_Asset *asset;
	f32 time;
	u32 current_frame;
};

enum Asset_ID {
	GUY1_ASSET,
	GUY2_ASSET,
};

enum Shader_Type {
	TEXTURED_STATIC_SHADER,
	UNTEXTURED_STATIC_SHADER,
	SHADOW_MAP_STATIC_SHADER,

	SHADER_COUNT
};

#include <assimp/cimport.h>
struct Material {
	aiString name; // @TODO: Remove name member.

	Shader_Type shader;

	V3 diffuse_color;
	V3 specular_color;

	Asset_ID diffuse_map;
	Asset_ID specular_map;
	Asset_ID normal_map;
};

// @TODO: Calculate based on available memory?
#define MAX_LOADED_ASSET_COUNT 1000

struct Game_Assets {
	Memory_Arena arena;

	Material materials[MAX_MATERIAL_COUNT];
	s32 material_count;

	Asset_Info<Animation_Asset, Animation_Instance> animations;
	Asset_Info<Model_Asset, Model_Instance> models;

	void *lookup[MAX_LOADED_ASSET_COUNT];
};

struct Game_State {
	Game_Execution_Status execution_status;
	Game_Input input;
	Game_Assets assets;
	Camera camera;

	Memory_Arena frame_arena;
	Memory_Arena permanant_arena;
};
