#pragma once

namespace path
{

struct Builder
{
	str::Builder stringBuilder;

	u8 &operator[](s64 i);
	const u8 &operator[](s64 i) const;
	bool operator==(Builder sb);
	bool operator!=(Builder sb);
	u8 *begin();
	u8 *end();
	s64 Length();
	str::String CopyString();
	str::String CopyStringIn(mem::Allocator *a);
	str::String MoveString();
	void Resize(s64 len);
	void Reserve(s64 reserve);
	void Append(View s);
	void AppendAll(arr::View<String> ss);
	void Format(String fmt, ...);
	void FormatVarArgs(str::View fmt, va_list args);
	void SetFilename(str::View name);
	void SetFilestem(str::View stem);
	void SetExtension(str::View ext);
	void SetDirectory(str::View dir);
}

template <typename... StringPack>
str::String JoinIn(mem::Allocator *a, StringPack... sp)
{
	auto n = sizeof...(sp);
	if (n == 0)
	{
		return;
	}
	auto len = (str::Length(sp) + ...) + (n - 1);
	auto pb = NewBuilderWithCapacityIn(a, len);
	(pb.Append(sp), ...);
	return pb.MoveString();
}

template <typename... StringPack>
str::String Join(StringPack... sp)
{
	return JoinIn(mem::ContextAllocator(), sp...);
}

}
