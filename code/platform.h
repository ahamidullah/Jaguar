#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <math.h>

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

#define U32_MAX UINT32_MAX

#define KILOBYTE(b) ((size_t)b*1024)
#define MEGABYTE(b) (KILOBYTE(b)*1024)
#define GIGABYTE(b) (MEGABYTE(b)*1024)

#define MILLISECONDS(t) (t * 1000)

#define ARRAY_COUNT(x) (sizeof(x)/sizeof(x[0]))

#define _abort(fmt, ...) _abort_actual(__FILE__, __LINE__, __func__, fmt, ## __VA_ARGS__)
#define log_print(log_type, fmt, ...) log_print_actual(log_type, __FILE__, __LINE__, __func__, fmt, ## __VA_ARGS__)

#ifdef DEBUG
	const u8 debug = 1;
#else
	const u8 debug = 0;
#endif

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

typedef enum {
	GAME_RUNNING,
	GAME_PAUSED,
	GAME_EXITING,
} Game_Execution_Status;
