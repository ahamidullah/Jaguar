#pragma once

namespace str
{

struct String;

// @TODO: Make this utf-8 by default?
struct Builder
{
	arr::Array<u8> buffer;

	u8 &operator[](s64 i);
	const u8 &operator[](s64 i) const;
	bool operator==(Builder sb);
	bool operator!=(Builder sb);
	u8 *begin();
	u8 *end();
	s64 Length();
	String ToString();
	String ToStringIn(mem::Allocator *a);
	String ToView(s64 start, s64 end);
	void Resize(s64 len);
	void Reserve(s64 reserve);
	void Append(String s);
	void AppendAll(arr::View<String> ss);
	void Format(String fmt, ...);
	void FormatVarArgs(String fmt, va_list args);
	void FormatTime();
	void Uppercase();
	void Lowercase();
	s64 FindFirst(u8 c);
	s64 FindLast(u8 c);
};

Builder NewBuilder(s64 len);
Builder NewBuilderIn(mem::Allocator *a, s64 len);
Builder NewBuilderWithCapacity(s64 cap);
Builder NewBuilderWithCapacityIn(mem::Allocator *a, s64 cap);

}
