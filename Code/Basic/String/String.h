#pragma once

#include "Basic/Container/Array.h"
#include "Common.h"

namespace string
{

struct String
{
	array::Array<u8> buffer;
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
	String CopyIn(Memory::Allocator *a);
	String CopyRange(s64 start, s64 end);
	String CopyRangeIn(Memory::Allocator *a, s64 start, s64 end);
	char *ToCString();
	String ToView(s64 start, s64 end);
	s64 FindFirst(u8 c);
	s64 FindLast(u8 c);
	array::Array<String> Split(char seperator);
};

String New(s64 len);
String NewIn(Memory::Allocator *a, s64 len);
String NewWithCapacity(s64 cap);
String NewWithCapacityIn(Memory::Allocator *a, s64 cap);
String NewFromBuffer(array::Array<u8> b);
String Make(const char *s);
String Make(char *s);
s64 ParseInt(String s, bool *err);
f32 ParseFloat(String s, bool *err);
String Format(String fmt, ...);
String FormatVarArgs(String fmt, va_list args);
bool Equal(const char *a, const char *b);
s64 Length(const char *s);
s64 Length(String s);

template <typename... StringPack>
String JoinIn(Memory::Allocator *a, StringPack... sp)
{
	auto n = sizeof...(sp);
	if (n == 0)
	{
		return New("");
	}
	auto len = (Length(sp) + ...);
	auto sb = NewBuilderWithCapacityIn(a, len);
	(sb.Append(sp), ...);
	return NewFromBuffer(sb.buffer);
}

template <typename... StringPack>
String Join(StringPack... sp)
{
	return JoinIn(Memory::ContextAllocator(), sp...);
}

}
