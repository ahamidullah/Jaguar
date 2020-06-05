#pragma once

#include "Array.h"

s64 CStringLength(const char *s);

// @TODO: Make this utf-8 by default?
struct String
{
	Array<char> data;
	s64 length;

	String() : String("") {};
	String(const Array<char> &a) : data{a} {}
	String(const char *cString) : data{CreateArray(CStringLength(cString) + 1, cString)} {};
	char &operator[](s64 i);
	char &operator[](s64 i) const;
};

bool operator==(const String &a, const String &b);
bool operator!=(const String &a, const String &b);

bool CStringsEqual(const char *a, const char *b);

String CreateString(s64 length);
String CreateString(s64 length, s64 capacity);
String CreateString(const String &copy);
String CreateString(const String &copy, s64 startIndex, s64 endIndex);

s64 StringLength(const String &s);

void ResizeString(String *s, s64 newSize);

void StringAppend(String *destination, const String &source);
void StringAppend(String *destination, const char *source);
void StringAppend(String *destination, String source, s64 rangeStartIndex, s64 rangeLength);
void StringAppend(String *destination, char source);

// @TODO: Change back to Concatenate.
String JoinStrings(const String &a, const String &b);

String FormatString(const String &format, ...);
String FormatStringVarArgs(const String &format, va_list arguments);

s64 FindFirstCharIndex(const String &s, char c);
s64 FindLastCharIndex(const String &s, char c);

bool IsCharWhitespace(char c);
bool IsCharDigit(char c);

void TrimString(String *s, s64 leftIndex, s64 rightIndex);
Array<String> SplitString(const String &s, char seperator);

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

void ArrayAppend(Array<String> *a, const char *newElement);
