#include "Builder.h"
#include "stb_sprintf.h"

namespace str
{

Builder NewBuilderIn(mem::Allocator *a, s64 len)
{
	auto sb = Builder
	{
		.buffer = arr::NewIn<u8>(a, len)
	};
	return sb;
}

Builder NewBuilder(s64 len)
{
	return NewBuilderIn(mem::ContextAllocator(), len);
}

Builder NewBuilderWithCapacityIn(mem::Allocator *a, s64 cap)
{
	auto sb = Builder
	{
		.buffer = arr::NewWithCapacityIn<u8>(a, cap)
	};
	return sb;
}

Builder NewBuilderWithCapacity(s64 cap)
{
	return NewBuilderWithCapacityIn(mem::ContextAllocator(), cap);
}

u8 &Builder::operator[](s64 i)
{
	Assert(i >= 0 && i < this->buffer.count);
	return this->buffer[i];
}

const u8 &Builder::operator[](s64 i) const
{
	Assert(i >= 0 && i < this->buffer.count);
	return this->buffer[i];
}

bool Builder::operator==(Builder sb)
{
	return this->buffer == sb.buffer;
}

bool Builder::operator!=(Builder sb)
{
	return this->buffer != sb.buffer;
}

u8 *Builder::begin()
{
	return this->buffer.begin();
}

u8 *Builder::end()
{
	return this->buffer.end();
}

s64 Builder::Length()
{
	return this->buffer.count;
}

String Builder::Build()
{
	return this->CopyStringIn(mem::ContextAllocator());
}

String Builder::BuildIn(mem::Allocator *a)
{
	auto buf = arr::NewIn<u8>(a, this->Length());
	arr::Copy(this->buffer, buf);
	return NewFromBuffer(buf);
}

View Builder::View(s64 start, s64 end)
{
	Assert(start <= end);
	Assert(end <= this->buffer.count);
	return
	{
		.buffer = this->buffer.View(start, this->buffer.Length());
	};
}

void Builder::Resize(s64 len)
{
	this->buffer.Resize(len);
}

void Builder::Append(String s)
{
	auto oldLen = this->Length();
	auto newLen = this->Length() + s.Length();
	this->Resize(newLen);
	arr::Copy(s.buffer, this->buffer.View(oldLen, newLen));
}

void Builder::AppendAll(arr::View<String> ss)
{
	auto len = 0;
	for (auto s : ss)
	{
		len += s.Length();
	}
	this->Reserve(len);
	for (auto s : ss)
	{
		this->Append(s);
	}
}

void Builder::Reserve(s64 reserve)
{
	this->buffer.Reserve(reserve);
}

void Builder::Format(String fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	this->FormatVarArgs(fmt, args);
	va_end(args);
}

void Builder::FormatVarArgs(String fmt, va_list args)
{
	this->Resize(this->Length() + STB_SPRINTF_MIN);
	auto Callback = [](char *, void *userData, s32 len) -> char *
	{
		auto sb = (Builder *)userData;
		sb->buffer.Resize(sb->Length() + STB_SPRINTF_MIN);
		return (char *)&sb->buffer[len];
	};
	auto cfmt = fmt.CString();
	auto len = stbsp_vsprintfcb(Callback, this, (char *)&this->buffer[0], cfmt, args);
	this->buffer.Resize(len);
}

void Builder::FormatTime()
{
	this->Append("%d-%d-%d %d:%d:%d.%d  ");
	// @TODO
}

void Builder::Uppercase()
{
	for (auto i = 0; i < this->Length(); i += 1)
	{
		(*this)[i] = UppercaseChar((*this)[i]);
	}
}

void Builder::Lowercase()
{
	for (auto i = 0; i < this->Length(); i += 1)
	{
		(*this)[i] = LowercaseChar((*this)[i]);
	}
}

s64 Builder::FindFirst(u8 c)
{
	for (auto i = 0; i < this->Length(); i += 1)
	{
		if ((*this)[i] == c)
		{
			return i;
		}
	}
	return -1;
}

s64 Builder::FindLast(u8 c)
{
	auto last = -1;
	for (auto i = 0; i < this->Length(); i += 1)
	{
		if ((*this)[i] == c)
		{
			last = i;
		}
	}
	return last;
}

}
