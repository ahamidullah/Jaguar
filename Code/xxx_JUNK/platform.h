#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>
#include <math.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef float f32;
typedef double f64;

#define U32_MAX UINT32_MAX

#define Kilobyte(b) ((size_t)b*1024)
#define Megabyte(b) (Kilobyte(b)*1024)
#define Gigabyte(b) (Megabyte(b)*1024)

#define Milliseconds(t) (t * 1000)

#define ArrayCount(x) (sizeof(x)/sizeof(x[0]))

void AssertActual(bool test, const char *fileName, const char *functionName, s32 lineNumber, const char *testName);
#define Assert(x) AssertActual(x, __FILE__, __func__, __LINE__, #x)

void Console_Print(const char *format, ...);
void Abort(const char *format, ...);

#define InvalidCodePath() Abort("%s: %s: line %d: invalid code path\n", __FILE__, __func__, __LINE__);

#if defined(DEBUG)
	const bool debug = 1;
#else
	const bool debug = 0;
#endif

struct IO_Buttons {
	u8 *down;
	u8 *pressed;
	u8 *released;
};

struct Mouse {
	s32 wheel;
	s32 x, y;
	s32 delta_x, delta_y;
	f32 raw_delta_x, raw_delta_y;
	f32 sensitivity;
	IO_Buttons buttons;
};

struct Game_Input {
	Mouse mouse;
	IO_Buttons keyboard;
};

enum GameExecutionStatus {
	GAME_RUNNING,
	GAME_PAUSED,
	GAME_EXITING,
};

