#pragma once

struct stringView {
	arrayView<u8> buffer;

	const u8 &operator[](s64 i);
	bool operator==(string s);
	bool operator!=(string s);
	bool operator==(const char *s);
	bool operator!=(const char *s);
	const u8 *begin();
	const u8 *end();
	s64 Length();
}

struct string {
	array<u8> buffer;
	bool literal;

	String();
	String(const char *s);
	String(char *s);
	const u8 &operator[](s64 i);
	bool operator==(string s);
	bool operator!=(string s);
	bool operator==(const char *s);
	bool operator!=(const char *s);
	const u8 *begin();
	const u8 *end();
	s64 Length();
	string Copy();
	string CopyIn(allocator *a);
	string CopyRange(s64 start, s64 end);
	string CopyRangeIn(allocator *a, s64 start, s64 end);
	char *CString();
	stringView View(s64 start, s64 end);
	arrayView<u8> Bytes();
	s64 FindFirst(u8 c);
	s64 FindLast(u8 c);
	array<string> Split(char seperator);
};

string New(s64 len);
string NewIn(allocator *a, s64 len);
string NewWithCapacity(s64 cap);
string NewWithCapacityIn(allocator *a, s64 cap);
string NewFromBuffer(array<u8> b);
string Make(const char *cs);
string Make(char *cs);
string Format(stringView fmt, ...);
string FormatVarArgs(stringView fmt, va_list args);
s64 ParseInt(stringView s, bool *err);
f32 ParseFloat(stringView s, bool *err);
s64 StringLength(stringView s);
s64 StringLength(const char *s);
bool StringEqual(const char *a, const char *b);
bool StringEqual(stringView a, stringView b);
bool StringEqual(const char *a, stringView b);
bool StringEqual(stringView a, const char *b);

struct stringBuilder;
stringBuilder NewStringBuilderWithCapacityIn(allocator *a, s64 cap);

template <typename... StringPack>
string JoinIn(allocator *a, StringPack... sp) {
	auto n = sizeof...(sp);
	if (n == 0) {
		return "";
	}
	auto len = (StringLength(sp) + ...);
	auto sb = NewStringBuilderWithCapacityIn(a, len);
	(sb.Append(sp), ...);
	return NewStringFromBuffer(sb.buffer);
}

template <typename... StringPack>
string Join(StringPack... sp) {
	return JoinIn(ContextAllocator(), sp...);
}

struct stringBuilder {
	array<u8> buffer;

	u8 &operator[](s64 i);
	const u8 &operator[](s64 i) const;
	bool operator==(stringBuilder sb);
	bool operator!=(stringBuilder sb);
	u8 *begin();
	u8 *end();
	s64 Length();
	string String();
	string StringIn(allocator *a);
	void Resize(s64 len);
	void Reserve(s64 reserve);
	void Append(stringView s);
	void AppendAll(arrayView<string> ss);
	void Format(string fmt, ...);
	void FormatVarArgs(string fmt, va_list args);
	void FormatTime();
	void Uppercase();
	void Lowercase();
	s64 FindFirst(u8 c);
	s64 FindLast(u8 c);
};

stringBuilder NewStringBuilder(s64 len);
stringBuilder NewStringBuilderIn(allocator *a, s64 len);
stringBuilder NewStringBuilderWithCapacity(s64 cap);
stringBuilder NewStringBuilderWithCapacityIn(allocator *a, s64 cap);

bool IsCharWhitespace(char c);
bool IsCharDigit(char c);
char UppercaseChar(char c);
char LowercaseChar(char c);
