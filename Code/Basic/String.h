#pragma once

#include "Memory.h"
#include "Array.h"

size_t Length(const char *s);

// @TODO: Make this utf-8 by default?
struct String
{
	Array<char> data;
	String() = default;
	String(const Array<char> &a) : data{a} {}
	String(const char *cString) : data{CreateArray(Length(cString) + 1, cString)} {};
	char &operator[](size_t i);
	char &operator[](size_t i) const;
};

bool operator==(const String &a, const String &b);
bool operator!=(const String &a, const String &b);
bool CStringsEqual(const char *a, const char *b);

String CreateString(size_t length, size_t capacity);
String CreateString(size_t length);
String CreateString(const String &copy);
String CreateString(const String &copy, size_t startIndex, size_t endIndex);
size_t Length(const String &s);
size_t Length(const String *s);
void Append(String *destination, const String &source);
void Append(String *destination, const char *source);
void Append(String *destination, String source, u32 rangeStartIndex, u32 rangeLength);
void Append(String *s, char c);
String _FormatString(const char *format, ...);
s32 FormatString(char *buffer, const char *format, va_list arguments);
s32 FormatString(char *buffer, const char *format, ...);
s64 FindFirstIndex(const String &s, char c);
s64 FindLastIndex(const String &s, char c);
bool IsSpace(char c);
void Trim(String *s, size_t leftIndex, size_t rightIndex);

template<typename... StringPack>
size_t Length(const String& S, const StringPack... rest) {
	return Length(S) + Length(rest...);
}

template<typename... StringPack>
size_t Length(const char *S, const StringPack... rest) {
	return Length(S) + Length(rest...);
}

template<typename... StringPack>
void ConcatenateAppend(size_t writeIndex, String *result, const String &source) {
	CopyMemory(&source.data[0], &result->data[writeIndex], Length(source));
}

template<typename... StringPack>
void ConcatenateAppend(size_t writeIndex, String *result, const char *source) {
	CopyMemory(source, &result->data[writeIndex], Length(source));
}

template<typename... StringPack>
void ConcatenateAppend(size_t writeIndex, String *result, const String &first, StringPack... rest) {
	CopyMemory(&first.data[0], &result->data[writeIndex], Length(first));
	ConcatenateAppend(writeIndex + Length(first), result, rest...);
}

template<typename... StringPack>
void ConcatenateAppend(size_t writeIndex, String *result, const char *first, StringPack... rest) {
	CopyMemory(first, &result->data, Length(first));
	ConcatenateAppend(writeIndex + Length(first), result, rest...);
}

template<typename T, typename... StringPack>
String Concatenate(const T &first, const StringPack... rest) {
	String result = CreateString(Length(first, rest...));
	ConcatenateAppend(0, &result, first, rest...);
	result.data[Length(result)] = '\0';
	return result;
}
