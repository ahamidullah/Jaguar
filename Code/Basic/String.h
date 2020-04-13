#pragma once

#include "Memory.h"
#include "Array.h"

size_t Length(const char *s);

// @TODO: Make this utf-8 by default?
struct String
{
	Array<char> data;
	String() : String("") {};
	String(const Array<char> &a) : data{a} {}
	String(const char *cString) : data{CreateArray(Length(cString) + 1, cString)} {};
	char &operator[](size_t i);
	char &operator[](size_t i) const;
};

bool operator==(const String &a, const String &b);
bool operator!=(const String &a, const String &b);
bool CStringsEqual(const char *a, const char *b);

String CreateString(size_t length, size_t capacity);
String CreateStringCopy(const String &copy);
String CreateStringCopyRange(const String &copy, size_t startIndex, size_t endIndex);

size_t StringLength(const String &s);

void ResizeString(String *s, size_t newSize);

void StringAppend(String *destination, const String &source);
void StringAppendCString(String *destination, const char *source);
void StringAppendRange(String *destination, String source, size_t rangeStartIndex, size_t rangeLength);

String FormatString(const char *format, ...);
s32 FormatStringBuffer(char *buffer, const char *format, ...);
s32 FormatStringBufferVarArgs(char *buffer, const char *format, va_list arguments);

s64 FindFirstCharIndex(const String &s, char c);
s64 FindLastCharIndex(const String &s, char c);

bool IsCharSpace(char c);
bool IsCharDigit(char c);

void TrimString(String *s, size_t leftIndex, size_t rightIndex);

bool ParseInteger(const String &string, s64 *result);

char UppercaseChar(char c);
char LowercaseChar(char c);
void UppercaseString(String *s);
void LowercaseString(String *s);

char *begin(const String &s);
char *end(const String &s);
char *begin(String *s);
char *end(String *s);

u64 Hash(const String &s);
