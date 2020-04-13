#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

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
#define U64_MAX UINT64_MAX

#define Kilobyte(b) ((size_t)b*1024)
#define Megabyte(b) (Kilobyte(b)*1024)
#define Gigabyte(b) (Megabyte(b)*1024)

#define Milliseconds(t) (t * 1000)

#define ArrayCount(x) (sizeof(x)/sizeof(x[0]))

#define InvalidCodePath() Abort("%s: %s: line %d: invalid code path\n", __FILE__, __func__, __LINE__);

#if defined(DEVELOPMENT)
	constexpr bool development = true;
#else
	constexpr bool development = false;
#endif

#if defined(DEBUG)
	constexpr bool debug = true;
#else
	constexpr bool debug = false;
#endif

template <typename F>
struct ScopeExit {
	ScopeExit(F _f) : f(_f) {}
	~ScopeExit() { f(); }
	F f;
};

template <typename F>
ScopeExit<F> MakeScopeExit(F f) {
	return ScopeExit<F>(f);
}

#define DO_STRING_JOIN(arg1, arg2) arg1 ## arg2
#define STRING_JOIN(arg1, arg2) DO_STRING_JOIN(arg1, arg2)
#define Defer(code) auto STRING_JOIN(scope_exit_, __LINE__) = MakeScopeExit([=](){code;})

#define Assert(x) AssertActual(x, __FILE__, __func__, __LINE__, #x)
void AssertActual(bool test, const char *fileName, const char *functionName, s32 lineNumber, const char *testName);

#define Abort(format, ...) AbortActual(format, __FILE__, __func__, __LINE__, ##__VA_ARGS__)
void AbortActual(const char *format, const char *fileName, const char *functionName, s32 lineNumber, ...);

#if defined(__linux__)
	#include "Linux/Log.h"
	#include "Linux/Time.h"
	#include "Linux/Memory.h"
	#include "Linux/Process.h"
	#include "Linux/File.h"
	#include "Linux/Thread.h"
	#include "Linux/Mutex.h"
	#include "Linux/Semaphore.h"
	#include "Linux/Atomic.h"
	#include "Linux/Fiber.h"
	#include "Linux/DLL.h"
#else
	#error unsupported platform
#endif

#include "Memory.h"
#include "Array.h"
#include "HashTable.h"
#include "String.h"
#include "File.h"
#include "Log.h"
#include "Filesystem.h"
#include "Parser.h"
//#include "RNG.h"

void InitializeBasic(u32 fiberCount = 0);
