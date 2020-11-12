#include "Builder.h"

namespace string
{

Builder NewBuilderIn(Memory::Allocator *a, s64 len)
{
	auto sb = Builder
	{
		.buffer = array::NewIn<u8>(a, len)
	};
	return sb;
}

Builder NewBuilder(s64 len)
{
	return NewBuilderIn(Memory::ContextAllocator(), len);
}

Builder NewBuilderWithCapacityIn(Memory::Allocator *a, s64 cap)
{
	auto sb = Builder
	{
		.buffer = array::NewWithCapacityIn<u8>(a, cap)
	};
	return sb;
}

Builder NewBuilderWithCapacity(s64 cap)
{
	return NewBuilderWithCapacityIn(Memory::ContextAllocator(), cap);
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

String Builder::ToString()
{
	return this->ToStringIn(Memory::ContextAllocator());
}

String Builder::ToView(s64 start, s64 end)
{
	Assert(start <= end);
	Assert(end <= this->buffer.count);
	auto b = array::Array<u8>
	{
		.allocator = Memory::NullAllocator(),
		.elements = &this->buffer.elements[start],
		.count = end - start,
	};
	return NewFromBuffer(b);
}

String Builder::ToStringIn(Memory::Allocator *a)
{
	auto buf = array::NewIn<u8>(a, this->Length());
	array::Copy(this->buffer, buf);
	return NewFromBuffer(buf);
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
	array::Copy(s.buffer, this->buffer.ToView(oldLen, newLen));
}

void Builder::AppendAll(array::View<String> ss)
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
	auto cfmt = fmt.ToCString();
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
