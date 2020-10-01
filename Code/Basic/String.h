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
typedef s32 Rune;

#if 0
struct StringView
{
	ArrayView<u8> buffer;

	StringView(const char *s, s64 len);
	StringView(const char *s);
	StringView(ArrayView<u8> a);
	s64 Length();
	const char *CString();
	String ToCopy(s64 start, s64 end);
	StringView ToView(s64 start, s64 end);
	s64 FindFirst(u8 c);
	s64 FindLast(u8 c);
	// @TODO:
	// operator[]
	// operator==
	// operator!=
	// begin
	// end
};
#endif

struct String
{
	Array<u8> buffer;
	bool literal;

	String();
	String(char *s);
	String(const char *s);
	String(Array<u8> buf);
	const u8 &operator[](s64 i);
	bool operator==(String s);
	bool operator!=(String s);
	const u8 *begin();
	const u8 *end();
	s64 Length();
	String Copy(s64 start, s64 end);
	char *CString();
	String View(s64 start, s64 end);
	s64 FindFirst(u8 c);
	s64 FindLast(u8 c);
	Array<String> Split(char seperator);
};

String NewString(const char *s);
String NewStringFromRange(String s, s64 start, s64 end);
String NewStringFromBuffer(Array<u8> b);
bool ParseInteger(String s, s64 *out);
String FormatString(String fmt, ...);

template <typename... StringPack>
String JoinStringsIn(Allocator *a, StringPack... sp)
{
	auto n = sizeof...(sp);
	if (n == 0)
	{
		return "";
	}
	auto len = (sp.Length() + ...);
	auto sb = NewStringBuilderWithCapacityIn(a, len);
	(sb->Append(sp), ...);
	return
	{
		.buffer = sb.buffer,
	};
}

template <typename... StringPack>
String JoinStrings(StringPack... sp)
{
	return JoinStringsIn(ContextAllocator(), sp...);
}

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
	struct String StringIn(Allocator *a);
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
StringBuilder NewStringBuilderIn(Allocator *a, s64 len);
StringBuilder NewStringBuilderWithCapacity(s64 cap);
StringBuilder NewStringBuilderWithCapacityIn(Allocator *a, s64 cap);
