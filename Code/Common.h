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
typedef intptr_t PointerInt;

const auto U8Max = UINT8_MAX;
const auto U32Max = UINT32_MAX;
const auto U64Max = UINT64_MAX;
const auto S32Max = INT32_MAX;
const auto S64Max = INT64_MAX;

const auto Kilobyte = 1024;
const auto Megabyte = 1024 * Kilobyte;
const auto Gigabyte = 1024 * Megabyte;

template <typename F>
struct ScopeExit
{
	ScopeExit(F _f) : f(_f) {}
	~ScopeExit() { f(); }
	F f;
};

template <typename F>
ScopeExit<F> NewScopeExit(F f)
{
	return ScopeExit<F>(f);
}

#define DO_STRING_JOIN(arg1, arg2) arg1 ## arg2
#define STRING_JOIN(arg1, arg2) DO_STRING_JOIN(arg1, arg2)
#define Defer(code) auto STRING_JOIN(scope_exit_, __LINE__) = NewScopeExit([=]() mutable {code;})
