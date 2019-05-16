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

enum Execution_State {
	RUNNING_STATE,
	PAUSED_STATE,
	EXITING_STATE,
};

enum Log_Type {
	STANDARD_LOG,
	MINOR_ERROR_LOG,
	MAJOR_ERROR_LOG,
	CATASTROPHIC_ERROR_LOG,
};

#define INVALID_CODE_PATH assert(!"Invalid code path.");

#define MILLISECONDS(t) (t * 1000)

#define KILOBYTE(b) ((size_t)b*1024)
#define MEGABYTE(b) (KILOBYTE(b)*1024)
#define GIGABYTE(b) (MEGABYTE(b)*1024)

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

struct V2 {
	f32 x, y;
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

typedef struct IO_Button {
	u8 down;
	u8 pressed;
	u8 released;
} IO_Button;

#define NUM_MOUSE_BUTTONS 3
struct Mouse {
	s32 wheel;
	V2 position;
	V2 delta_position;
	f32 sensitivity;
	IO_Button buttons[NUM_MOUSE_BUTTONS];
};

#define MAX_SCANCODES 256
struct Input {
	Mouse mouse;
	IO_Button keyboard[MAX_SCANCODES];
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
	s32 num_bytes;
	T *data;

	T &operator[](s32 i);
};

