#include "String.h"
#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"
#undef STB_SPRINTF_IMPLEMENTATION

string() {
	this->buffer = {};
	this->literal = false;
}

string(const char *s) {
	*this = MakeString(s);
}

string(char *s) {
	*this = MakeString(s);
}

string NewString(s64 len) {
	auto s = string{};
	s.buffer = NewArray<u8>(len);
	s.literal = false;
	return s;
}

string NewStringIn(allocator *a, s64 len) {
	auto s = string{};
	s.buffer = NewArrayIn<u8>(a, len);
	s.literal = false;
	return s;
}

string NewStringWithCapacity(s64 cap) {
	auto s = string{};
	s.buffer = NewArrayWithCapacity<u8>(cap);
	s.literal = false;
	return s;
}

string NewStringWithCapacityIn(allocator *a, s64 cap) {
	auto s = string{};
	s.buffer = NewArrayWithCapacityIn<u8>(a, cap);
	s.literal = false;
	return s;
}

string NewStringFromBuffer(array<u8> b) {
	auto s = string{};
	s.buffer = b;
	s.literal = false;
	return s;
}

string MakeString(const char *cs) {
	auto len = Length(cs);
	auto s = string{};
	s.buffer = array<u8>{
		.allocator = NullAllocator(),
		.elements = (u8 *)cs,
		.count = len,
		.capacity = len,
	};
	s.literal = true;
	return s;
}

string MakeString(char *cs) {
	auto len = Length(cs);
	auto s = string{};
	s.buffer = NewArray<u8>(len);
	CopyArray(NewArrayView((u8 *)cs, len), s.buffer);
	s.literal = false;
	return s;
}

const u8 &string::operator[](s64 i) {
	Assert(i >= 0 && i < this->Length());
	return this->buffer[i];
}

bool string::operator==(string s) {
	if (this->Length() != s.Length()) {
		return false;
	}
	for (auto i = 0; i < this->Length(); i += 1) {
		if (this->buffer[i] != s.buffer[i]) {
			return false;
		}
	}
	return true;
}

bool string::operator!=(string s) {
	return !(s == *this);
}

bool string::operator==(const char *s) {
	if (this->Length() != StringLength(s)) {
		return false;
	}
	for (auto i = 0; i < this->Length(); i += 1) {
		if (this->buffer[i] != s[i]) {
			return false;
		}
	}
	return true;
}

bool string::operator!=(const char *s) {
	return !(s == *this);
}

const u8 *string::begin() {
	return this->buffer.begin();
}

const u8 *string::end() {
	return this->buffer.end();
}

s64 string::Length() {
	return this->buffer.count;
}

string string::Copy()
{
	return this->CopyIn(ContextAllocator());
}

string string::CopyIn(allocator *a)
{
	return NewFromBuffer(this->buffer.CopyIn(a));
}

string string::CopyRange(s64 start, s64 end)
{
	return this->CopyRangeIn(ContextAllocator(), start, end);
}

string string::CopyRangeIn(allocator *a, s64 start, s64 end)
{
	return NewFromBuffer(this->buffer.CopyRangeIn(a, start, end));
}

char *string::CString() {
	if (this->literal) {
		return (char *)this->buffer.elements;
	}
	auto s = (u8 *)AllocateMemory(this->Length() + 1);
	CopyArray(this->buffer, NewArrayView(s, this->Length()));
	s[this->Length()] = '\0';
	return (char *)s;
}

string string::View(s64 start, s64 end) {
	Assert(start <= end);
	auto buf = array<u8>{
		.allocator = NullAllocator(),
		.elements = &this->buffer[start],
		.count = end - start,
	};
	return NewStringFromBuffer(buf);
}

s64 string::FindFirst(u8 c) {
	return this->buffer.FindFirst(c);
}

s64 string::FindLast(u8 c) {
	return this->buffer.FindLast(c);
}

array<string> string::Split(char seperator) {
	auto splits = array<string>{};
	auto curStart = 0;
	auto curLen = 0;
	for (auto i = 0; i < this->Length(); i += 1) {
		if ((*this)[i] == seperator) {
			if (curLen > 0) {
				splits.Append(this->CopyRange(curStart, i - 1));
				curLen = 0;
			}
		} else {
			if (curLen == 0) {
				curStart = i;
			}
			curLen += 1;
			;
		}
	}
	if (curLen > 0) {
		splits.Append(this->CopyRange(curStart, this->Length() - 1));
	}
	return splits;
}

string Format(string fmt, ...) {
	va_list args;
	va_start(args, fmt);
	auto sb = Builder{};
	sb.FormatVarArgs(fmt, args);
	va_end(args);
	return NewStringFromBuffer(sb.buffer);
}

string FormatVarArgs(string fmt, va_list args) {
	auto sb = Builder{};
	sb.FormatVarArgs(fmt, args);
	return NewStringFromBuffer(sb.buffer);
}

s64 ParseInt(string s, bool *err) {
	auto n = 0;
	for (auto c : s) {
		if (!IsCharDigit(c)) {
			*err = true;
			return 0;
		}
		n = (n * 10) + c - '0';
	}
	return n;
}

f32 ParseFloat(string s, bool *err) {
	auto cs = s.CString();
	auto end = (char *){};
	auto f = strtof(cs, &end);
	if (end != &cs[s.Length()]) {
		*err = true;
	}
	return f;
}

s64 StringLength(string s) {
	return s.Length();
}

void CopyString(string src, string dst) {
	dst.Resize(src.Length());
	CopyArray(src.buffer, dst.buffer);
}

s64 StringLength(const char *s) {
	auto len = 0;
	while (s[len]) {
		len += 1;
	}
	return len;
}

bool StringEqual(const char *a, const char *b) {
	while (*a && *b) {
		if (*a != *b) {
			return false;
		}
		a += 1;
		b += 1;
	}
	if (*a || *b) {
		return false;
	}
	return true;
}
