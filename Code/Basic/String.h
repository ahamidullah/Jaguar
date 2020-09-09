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
	const char *ToCString();
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
	//operator StringView();
	const u8 &operator[](s64 i);
	bool operator==(String s);
	bool operator!=(String s);
	const u8 *begin();
	const u8 *end();
	s64 Length();
	String ToCopy(s64 start, s64 end);
	char *ToCString();
	String ToView(s64 start, s64 end);
	void Free();
	s64 FindFirst(u8 c);
	s64 FindLast(u8 c);
	Array<String> Split(char seperator);
	u64 Hash();
};

String NewString(const char *s);
String NewStringFromBuffer(Array<u8> b);
String FormatString(String fmt, ...);
bool ParseInteger(String s, s64 *out);

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

	//operator StringView();
	u8 &operator[](s64 i);
	const u8 &operator[](s64 i) const;
	bool operator==(StringBuilder sb);
	bool operator!=(StringBuilder sb);
	u8 *begin();
	u8 *end();
	s64 Length();
	String ToView(s64 start, s64 end);
	String StringIn(Allocator *a);
	String ToString();
	void Free();
	void Resize(s64 len);
	void Reserve(s64 reserve);
	void Append(String s);
	void AppendAll(ArrayView<String> ss);
	void Format(String fmt, ...);
	void FormatVarArgs(String fmt, va_list args);
	void FormatTime();
	void Uppercase();
	void Lowercase();
	s64 FindFirst(u8 c);
	s64 FindLast(u8 c);
};

StringBuilder NewStringBuilder(s64 len);
StringBuilder NewStringBuilderIn(Allocator *a, s64 len);
StringBuilder NewStringBuilderWithCapacity(s64 len, s64 cap);
StringBuilder NewStringBuilderWithCapacityIn(Allocator *a, s64 len, s64 cap);
