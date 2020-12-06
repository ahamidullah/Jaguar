#pragma once

#include "basic/container/array.h"
#include "Common.h"

namespace string
{

struct String
{
	arr::array<u8> buffer;
	bool literal;

	String();
	String(const char *s);
	String(char *s);
	const u8 &operator[](s64 i);
	bool operator==(String s);
	bool operator!=(String s);
	bool operator==(const char *s);
	bool operator!=(const char *s);
	const u8 *begin();
	const u8 *end();
	s64 Length();
	String Copy();
	String CopyIn(mem::allocator *a);
	String CopyRange(s64 start, s64 end);
	String CopyRangeIn(mem::allocator *a, s64 start, s64 end);
	char *CString();
	View View(s64 start, s64 end);
	arr::array<u8> Bytes();
	s64 FindFirst(u8 c);
	s64 FindLast(u8 c);
	arr::array<string> Split(char seperator);
};

String New(s64 len);
String NewIn(mem::Allocator *a, s64 len);
String NewWithCapacity(s64 cap);
String NewWithCapacityIn(mem::Allocator *a, s64 cap);
String NewFromBuffer(arr::array<u8> b);
String Make(const char *cs);
String Make(char *cs);
String Format(String fmt, ...);
String FormatVarArgs(String fmt, va_list args);

struct Builder;
Builder NewBuilderWithCapacityIn(mem::Allocator *a, s64 cap);

template <typename... StringPack>
String JoinIn(mem::Allocator *a, StringPack... sp)
{
	auto n = sizeof...(sp);
	if (n == 0)
	{
		return "";
	}
	auto len = (Length(sp) + ...);
	auto sb = NewBuilderWithCapacityIn(a, len);
	(sb.Append(sp), ...);
	return NewFromBuffer(sb.buffer);
}

template <typename... StringPack>
String Join(StringPack... sp)
{
	return JoinIn(mem::ContextAllocator(), sp...);
}

s64 ParseInt(String s, bool *err);
f32 ParseFloat(String s, bool *err);
s64 Length(String s);
s64 Length(const char *s);
bool Equal(const char *a, const char *b);
bool Equal(String a, String b);
bool Equal(const char *a, String b);
bool Equal(String a, const char *b);

}
