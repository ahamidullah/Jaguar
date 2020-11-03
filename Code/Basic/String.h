#pragma once

#include "Array.h"
#include "Common.h"

bool CStringsEqual(const char *a, const char *b);
s64 CStringLength(const char *s);
bool IsCharWhitespace(char c);
bool IsCharDigit(char c);
char UppercaseChar(char c);
char LowercaseChar(char c);

struct String;

// @TODO: Make this utf-8 by default?
struct StringBuilder
{
	Array<u8> buffer;

	u8 &operator[](s64 i);
	const u8 &operator[](s64 i) const;
	bool operator==(StringBuilder sb);
	bool operator!=(StringBuilder sb);
	u8 *begin();
	u8 *end();
	s64 Length();
	struct String String();
	struct String StringIn(Memory::Allocator *a);
	struct String View(s64 start, s64 end);
	void Resize(s64 len);
	void Reserve(s64 reserve);
	void Append(struct String s);
	void AppendAll(ArrayView<struct String> ss);
	void Format(struct String fmt, ...);
	void FormatVarArgs(struct String fmt, va_list args);
	void FormatTime();
	void Uppercase();
	void Lowercase();
	s64 FindFirst(u8 c);
	s64 FindLast(u8 c);
};

StringBuilder NewStringBuilder(s64 len);
StringBuilder NewStringBuilderIn(Memory::Allocator *a, s64 len);
StringBuilder NewStringBuilderWithCapacity(s64 cap);
StringBuilder NewStringBuilderWithCapacityIn(Memory::Allocator *a, s64 cap);

struct String
{
	Array<u8> buffer;
	bool literal;

	String();
	String(char *s); // @TODO: Get rid of this?
	String(const char *s);
	const u8 &operator[](s64 i);
	bool operator==(String s);
	bool operator!=(String s);
	const u8 *begin();
	const u8 *end();
	s64 Length();
	String Copy();
	String CopyIn(Memory::Allocator *a);
	String CopyRange(s64 start, s64 end);
	String CopyRangeIn(Memory::Allocator *a, s64 start, s64 end);
	char *CString();
	String View(s64 start, s64 end);
	s64 FindFirst(u8 c);
	s64 FindLast(u8 c);
	Array<String> Split(char seperator);
};

String NewString(const char *s);
String NewStringFromBuffer(Array<u8> b);
s64 ParseInteger(String s, bool *err);
s64 ParseIntegerAbort(String s);
f32 ParseFloat(String s, bool *err);
f32 ParseFloatAbort(String s);
String FormatString(String fmt, ...);
String FormatStringVarArgs(String fmt, va_list args);

template <typename... StringPack>
String JoinStringsIn(Memory::Allocator *a, StringPack... sp)
{
	auto n = sizeof...(sp);
	if (n == 0)
	{
		return "";
	}
	auto len = (String{sp}.Length() + ...);
	auto sb = NewStringBuilderWithCapacityIn(a, len);
	(sb.Append(sp), ...);
	return NewStringFromBuffer(sb.buffer);
}

template <typename... StringPack>
String JoinStrings(StringPack... sp)
{
	return JoinStringsIn(Memory::ContextAllocator(), sp...);
}
