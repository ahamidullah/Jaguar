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
typedef intptr_t IntegerPointer;

const auto U8Max = UINT8_MAX;
const auto U32Max = UINT32_MAX;
const auto U64Max = UINT64_MAX;
const auto S32Max = INT32_MAX;
const auto S64Max = INT64_MAX;
//const auto IntMax = intmax_t;
//const auto UintMax = uintmax_t;

#define KilobytesToBytes(k) ((u32)k * 1024)
#define MegabytesToBytes(m) (KilobytesToBytes(m) * 1024)
#define GigabytesToBytes(g) (MegabyteToBytes(g) * 1024)
#define BytesToKilobytes(k) (k / 1024.0)
#define BytesToMegabytes(m) (BytesToKilobytes(m) / 1024.0)
#define BytesToGigabytes(g) (BytesToMegabytes(g) / 1024.0)
#define SecondsToMilliseconds(t) (t * 1000)
//#define CArrayCount(x) (sizeof(x)/sizeof(x[0]))

template <typename F>
struct ScopeExit
{
	ScopeExit(F _f) : f(_f) {}
	~ScopeExit() { f(); }
	F f;
};

template <typename F>
ScopeExit<F> CreateScopeExit(F f)
{
	return ScopeExit<F>(f);
}

#define DO_STRING_JOIN(arg1, arg2) arg1 ## arg2
#define STRING_JOIN(arg1, arg2) DO_STRING_JOIN(arg1, arg2)
#define Defer(code) auto STRING_JOIN(scope_exit_, __LINE__) = CreateScopeExit([=]() mutable {code;})
