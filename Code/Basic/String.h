#pragma once

s64 CStringLength(const char *s);

// @TODO: CreateString with allocator.
// @TODO: Make this utf-8 by default?
struct String
{
	Array<char> buffer;
	s64 length;

	String() : String("") {};
	String(Array<char> a) : buffer{a} {}
	String(const char *s) : buffer{NewArrayFromData(s, CStringLength(s) + 1)} {};
	char &operator[](s64 i);
	char &operator[](s64 i) const;
};

bool operator==(const String &a, const String &b);
bool operator!=(const String &a, const String &b);

bool CStringsEqual(const char *a, const char *b);

String NewString(s64 length);
String NewStringIn(s64 length, AllocatorInterface a);
String NewStringWithCapacity(s64 length, s64 capacity);
String NewStringWithCapacityIn(s64 length, s64 capacity, AllocatorInterface a);
String NewStringCopy(String s);
String NewStringCopyIn(String s, AllocatorInterface a);
String NewStringCopyRange(String s, s64 start, s64 end);
String NewStringCopyRangeIn(String s, s64 start, s64 end, AllocatorInterface a);

s64 StringLength(String s);

void ResizeString(String *s, s64 newSize);

void AppendToString(String *d, String s);
void AppendDataToString(String *d, const char *s);
void AppendRangeToString(String *d, String s, s64 start, s64 length);
void AppendCharToString(String *d, char s);

// @TODO: Change back to Concatenate.
String JoinStrings(String a, String b);

String FormatString(String format, ...);
String FormatStringVarArgs(String format, va_list arguments);

s64 FindFirstChar(String s, char c);
s64 FindLastChar(String s, char c);

bool IsCharWhitespace(char c);
bool IsCharDigit(char c);

void TrimString(String *s, s64 left, s64 right);

Array<String> SplitString(String s, char seperator);

bool ParseInteger(String string, s64 *result);

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

template <typename... StringPack>
String JoinStrings(StringPack... strings)
{
	auto stringCount = sizeof...(strings);
	if (stringCount == 0)
	{
		return "";
	}
	auto totalStringLengths = (strings.count + ...);
	auto result = CreateString(0, totalStringLengths);
	(StringAppend(&result, strings), ...);
	return result;
}
