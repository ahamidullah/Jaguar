#pragma once

#include "Array.h"

#include "Code/Common.h"

bool CStringsEqual(const char *a, const char *b);
s64 CStringLength(const char *s);

bool IsCharWhitespace(char c);
bool IsCharDigit(char c);

char UppercaseChar(char c);
char LowercaseChar(char c);

struct String
{
	AllocatorInterface allocator;
	const char *buffer;
	s64 length;
	bool literal;

	String();
	String(char *s);
	String(char *s, s64 len);
	String(const char *s, s64 len);
	String(const char *s);
	String(AllocatorInterface a, const char *b, s64 len, bool lit);
	const char &operator[](s64 i) const;
};

bool operator==(String a, String b);
bool operator!=(String a, String b);

const char *begin(String s);
const char *end(String s);
const char *begin(String *s);
const char *end(String *s);

// @TODO: In versions of these functions.
String NewStringCopy(String src);
String NewStringCopyRange(String src, s64 start, s64 end);

s64 FindFirstCharInString(String s, char c);
s64 FindLastCharInString(String s, char c);

Array<String> SplitString(String s, char seperator);

bool ParseInteger(String string, s64 *result);

u64 HashString(String s);

void FreeString(String *s);

void AppendToArray(Array<String> *a, const char *s);

// @TODO: Make this utf-8 by default?
struct StringBuilder
{
	Array<char> buffer;
	s64 length; // Length of the string not including the null terminator.

	char &operator[](s64 i);
	const char &operator[](s64 i) const;
};

char *begin(StringBuilder sb);
char *end(StringBuilder sb);
char *begin(StringBuilder *sb);
char *end(StringBuilder *sb);

StringBuilder NewStringBuilder(s64 len);
StringBuilder NewStringBuilderIn(s64 len, AllocatorInterface a);
StringBuilder NewStringBuilderWithCapacity(s64 len, s64 cap);
StringBuilder NewStringBuilderWithCapacityIn(s64 len, s64 cap, AllocatorInterface a);
StringBuilder NewStringBuilderFromString(String s);
StringBuilder NewStringBuilderFromStringIn(String s, AllocatorInterface a);

void ClearStringBuilder(StringBuilder *sb);
void FreeStringBuilder(StringBuilder *sb);

String BuilderToString(StringBuilder sb);

void ResizeStringBuilder(StringBuilder *sb, s64 len);
void ReserveStringBuilder(StringBuilder *sb, s64 reserve);

void AppendToStringBuilder(StringBuilder *sb, String s);
void AppendRangeToStringBuilder(StringBuilder *sb, String s, s64 start, s64 end);

void FormatString(StringBuilder *sb, String fmt, ...);
void FormatStringVarArgs(StringBuilder *sb, String fmt, va_list arguments);

//void TrimStringBuilder(String *s, s64 left, s64 right);

void UppercaseStringBuilder(StringBuilder *sb);
void LowercaseStringBuilder(StringBuilder *sb);

s64 FindFirstCharInStringBuilder(StringBuilder sb, char c);
s64 FindLastCharInStringBuilder(StringBuilder sb, char c);

template <typename... StringPack>
String JoinStrings(StringPack... sp)
{
	auto n = sizeof...(sp);
	if (n == 0)
	{
		return "";
	}
	auto len = (sp.length + ...);
	auto buf = (char *)AllocateMemory(len + 1);
	auto i = 0;
	auto CopyString = [&i](char *b, String s)
	{
		CopyMemory(s.buffer, &b[i], s.length);
		i += s.length;
	};
	(CopyString(buf, sp), ...);
	buf[i] = '\0';
	return
	{
		.allocator = ContextAllocator(),
		.buffer = buf,
		.length = len,
		.literal = false,
	};
}
