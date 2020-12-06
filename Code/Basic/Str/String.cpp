#include "String.h"
#include "Basic/Memory.h"
#include "Basic/Log.h"
#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"
#undef STB_SPRINTF_IMPLEMENTATION

namespace str
{

str::String()
{
	this->buffer = {};
	this->literal = false;
}

str::String(const char *s)
{
	*this = Make(s);
}

str::String(char *s)
{
	*this = Make(s);
}

String New(s64 len)
{
	auto s = String{};
	s.buffer = arr::New<u8>(len);
	s.literal = false;
	return s;
}

String NewIn(mem::Allocator *a, s64 len)
{
	auto s = String{};
	s.buffer = arr::NewIn<u8>(a, len);
	s.literal = false;
	return s;
}

String NewWithCapacity(s64 cap)
{
	auto s = String{};
	s.buffer = arr::NewWithCapacity<u8>(cap);
	s.literal = false;
	return s;
}

String NewWithCapacityIn(mem::Allocator *a, s64 cap)
{
	auto s = String{};
	s.buffer = arr::NewWithCapacityIn<u8>(a, cap);
	s.literal = false;
	return s;
}

String NewFromBuffer(arr::array<u8> b)
{
	auto s = String{};
	s.buffer = b;
	s.literal = false;
	return s;
}

String Make(const char *cs)
{
	auto len = Length(cs);
	auto s = String{};
	s.buffer = arr::array<u8>
	{
		.allocator = mem::NullAllocator(),
		.elements = (u8 *)cs,
		.count = len,
		.capacity = len,
	};
	s.literal = true;
	return s;
}

String Make(char *cs)
{
	auto len = Length(cs);
	auto s = String{};
	s.buffer = arr::New<u8>(len);
	arr::Copy(arr::NewView((u8 *)cs, len), s.buffer);
	s.literal = false;
	return s;
}

const u8 &str::operator[](s64 i)
{
	Assert(i >= 0 && i < this->Length());
	return this->buffer[i];
}

bool str::operator==(String s)
{
	if (this->Length() != s.Length())
	{
		return false;
	}
	for (auto i = 0; i < this->Length(); i += 1)
	{
		if (this->buffer[i] != s.buffer[i])
		{
			return false;
		}
	}
	return true;
}

bool str::operator!=(String s)
{
	return !(s == *this);
}

bool str::operator==(const char *s)
{
	if (this->Length() != str::Length(s))
	{
		return false;
	}
	for (auto i = 0; i < this->Length(); i += 1)
	{
		if (this->buffer[i] != s[i])
		{
			return false;
		}
	}
	return true;
}

bool str::operator!=(const char *s)
{
	return !(s == *this);
}

const u8 *str::begin()
{
	return this->buffer.begin();
}

const u8 *str::end()
{
	return this->buffer.end();
}

s64 str::Length()
{
	return this->buffer.count;
}

String str::Copy()
{
	return this->CopyIn(mem::ContextAllocator());
}

String str::CopyIn(mem::Allocator *a)
{
	return NewFromBuffer(this->buffer.CopyIn(a));
}

String str::CopyRange(s64 start, s64 end)
{
	return this->CopyRangeIn(mem::ContextAllocator(), start, end);
}

String str::CopyRangeIn(mem::Allocator *a, s64 start, s64 end)
{
	return NewFromBuffer(this->buffer.CopyRangeIn(a, start, end));
}

char *str::CString()
{
	if (this->literal)
	{
		return (char *)this->buffer.elements;
	}
	auto s = (u8 *)mem::Allocate(this->Length() + 1);
	arr::Copy(this->buffer, arr::NewView(s, this->Length()));
	s[this->Length()] = '\0';
	return (char *)s;
}

String str::View(s64 start, s64 end)
{
	Assert(start <= end);
	auto buf = arr::array<u8>
	{
		.allocator = mem::NullAllocator(),
		.elements = &this->buffer[start],
		.count = end - start,
	};
	return NewFromBuffer(buf);
}

s64 str::FindFirst(u8 c)
{
	return this->buffer.FindFirst(c);
}

s64 str::FindLast(u8 c)
{
	return this->buffer.FindLast(c);
}

arr::array<String> str::Split(char seperator)
{
	auto splits = arr::array<String>{};
	auto curStart = 0;
	auto curLen = 0;
	for (auto i = 0; i < this->Length(); i += 1)
	{
		if ((*this)[i] == seperator)
		{
			if (curLen > 0)
			{
				splits.Append(this->CopyRange(curStart, i - 1));
				curLen = 0;
			}
		}
		else
		{
			if (curLen == 0)
			{
				curStart = i;
			}
			curLen += 1;;
		}
	}
	if (curLen > 0)
	{
		splits.Append(this->CopyRange(curStart, this->Length() - 1));
	}
	return splits;
}

String Format(String fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	auto sb = Builder{};
	sb.FormatVarArgs(fmt, args);
	va_end(args);
	return NewFromBuffer(sb.buffer);
}

String FormatVarArgs(String fmt, va_list args)
{
	auto sb = Builder{};
	sb.FormatVarArgs(fmt, args);
	return NewFromBuffer(sb.buffer);
}

s64 ParseInt(String s, bool *err)
{
    auto n = 0;
    for (auto c : s)
    {
    	if (!IsCharDigit(c))
    	{
    		*err = true;
    		return 0;
    	}
    	n = (n * 10) + c - '0';
    }
    return n;
}

f32 ParseFloat(String s, bool *err)
{
	auto cs = s.CString();
	auto end = (char *){};
	auto f = strtof(cs, &end);
	if (end != &cs[s.Length()])
	{
		*err = true;
	}
	return f;
}

s64 Length(String s)
{
	return s.Length();
}

s64 Length(const char *s)
{
	auto len = 0;
	while (s[len])
	{
		len += 1;
	}
	return len;
}

bool Equal(const char *a, const char *b)
{
	while (*a && *b)
	{
		if (*a != *b)
		{
			return false;
		}
		a += 1;
		b += 1;
	}
	if (*a || *b)
	{
		return false;
	}
	return true;
}

}
