#include "String.h"
#include "Basic/Memory.h"
#include "../Log.h"
#define STB_SPRINTF_IMPLEMENTATION
	#include "stb_sprintf.h"
#undef STB_SPRINTF_IMPLEMENTATION

namespace string
{

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

s64 Length(const char *s)
{
	auto len = 0;
	while (s[len])
	{
		len += 1;
	}
	return len;
}

s64 Length(String s)
{
	return s.length;
}

String New(s64 len)
{
	return
	{
		.buffer = array::New<u8>(len),
		.literal = false,
	};
}

String NewIn(Memory::Allocator *a, s64 len)
{
	return
	{
		.buffer = array::NewIn<u8>(a, len),
		.literal = false,
	};
}

String NewWithCapacity(s64 cap)
{
	return
	{
		.buffer = array::NewWithCapacity<u8>(cap),
		.literal = false,
	};
}

String NewWithCapacityIn(Memory::Allocator *a, s64 cap)
{
	return
	{
		.buffer = array::NewWithCapacityIn<u8>(a, cap),
		.literal = false,
	};
}

String NewFromBuffer(array::Array<u8> b)
{
	return
	{
		.buffer = b,
		.literal = false,
	};
}

String Make(const char *s)
{
	auto len = Length(s);
	return
	{
		.buffer = array::Array<u8>
		{
			.allocator = Memory::NullAllocator(),
			.elements = (u8 *)s,
			.count = len,
			.capacity = len,
		},
		.literal = true,
	};
}

const u8 &String::operator[](s64 i)
{
	Assert(i >= 0 && i < this->Length());
	return this->buffer[i];
}

bool String::operator==(String s)
{
	if (s.Length() != this->Length())
	{
		return false;
	}
	for (auto i = 0; i < s.Length(); i += 1)
	{
		if (s.buffer[i] != this->buffer[i])
		{
			return false;
		}
	}
	return true;
}

bool String::operator!=(String s)
{
	return !(s == *this);
}

bool String::operator==(const char *s)
{
	auto len = Length(s);
	if (len != this->Length())
	{
		return false;
	}
	for (auto i = 0; i < s.Length(); i += 1)
	{
		if (s[i] != this->buffer[i])
		{
			return false;
		}
	}
	return true;
}

bool String::operator!=(const char *s)
{
	return !(s == *this);
}

const u8 *String::begin()
{
	return this->buffer.begin();
}

const u8 *String::end()
{
	return this->buffer.end();
}

s64 String::Length()
{
	return this->buffer.count;
}

String String::Copy()
{
	return this->CopyIn(Memory::ContextAllocator());
}

String String::CopyIn(Memory::Allocator *a)
{
	return NewFromBuffer(this->buffer.CopyIn(a));
}

String String::CopyRange(s64 start, s64 end)
{
	return this->CopyRangeIn(Memory::ContextAllocator(), start, end);
}

String String::CopyRangeIn(Memory::Allocator *a, s64 start, s64 end)
{
	return return NewFromBuffer(this->buffer.CopyRangeIn(a, start, end));
}

char *String::ToCString()
{
	if (this->literal)
	{
		return (char *)this->buffer.elements;
	}
	auto s = (u8 *)Memory::Allocate(this->Length() + 1);
	array::Copy(this->buffer, array::NewView(s, this->Length()));
	s[this->Length()] = '\0';
	return (char *)s;
}

String String::ToView(s64 start, s64 end)
{
	Assert(start <= end);
	auto buf = array::Array<u8>
	{
		.allocator = Memory::NullAllocator(),
		.elements = &this->buffer[start],
		.count = end - start,
	};
	return NewFromBuffer(buf);
}

s64 String::FindFirst(u8 c)
{
	return this->buffer.FindFirst(c);
}

s64 String::FindLast(u8 c)
{
	return this->buffer.FindLast(c);
}

array::Array<String> String::Split(char seperator)
{
	auto splits = array::Array<String>{};
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

s64 ParseInteger(String s, bool *err)
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
	auto cs = s.ToCString();
	auto end = (char *){};
	auto f = strtof(cs, &end);
	if (end != &cs[s.Length()])
	{
		*err = true;
	}
	return f;
}

}
