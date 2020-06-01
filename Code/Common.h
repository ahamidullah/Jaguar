#pragma once

#include <stdint.h>

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
#define S32_MAX INT32_MAX
#define S64_MAX INT64_MAX

#define Kilobyte(b) ((size_t)b*1024)
#define Megabyte(b) (Kilobyte(b)*1024)
#define Gigabyte(b) (Megabyte(b)*1024)

#define Milliseconds(t) (t * 1000)

#define CArrayCount(x) (sizeof(x)/sizeof(x[0]))

#if defined(DEVELOPMENT)
	constexpr auto development = true;
#else
	constexpr auto development = false;
#endif

#if defined(DEBUG)
	constexpr auto debug = true;
#else
	constexpr auto debug = false;
#endif

template <typename F>
struct ScopeExit
{
	ScopeExit(F _f) : f(_f) {}
	~ScopeExit() { f(); }
	F f;
};

template <typename F>
ScopeExit<F> MakeScopeExit(F f)
{
	return ScopeExit<F>(f);
}

#define DO_STRING_JOIN(arg1, arg2) arg1 ## arg2
#define STRING_JOIN(arg1, arg2) DO_STRING_JOIN(arg1, arg2)
#define Defer(code) auto STRING_JOIN(scope_exit_, __LINE__) = MakeScopeExit([=](){code;})
