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

//Platform_Print_Stacktrace();
//Platform_Signal_Debug_Breakpoint();

#if defined(DEBUG)
#define Assert(x) \
	do { \
		if (!(x)) { \
			printf("%s: %s: line %d: assertion failed '%s'\n", __FILE__, __func__, __LINE__, #x); \
			s32 i = 0/0;
		} \
	} while(0)
#else
#define Assert(x)
#endif

//void ConsolePrint(const char *format, ...);
//void Abort(const char *format, ...);
