#include "String.h"
#include "Memory.h"
#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"
#undef STB_SPRINTF_IMPLEMENTATION

bool CStringsEqual(const char *a, const char *b)
{
	while (*a && *b)
	{
		if (*a != *b)
		{
			return false;
		}
		a++;
		b++;
	}
	if (*a || *b)
	{
		return false;
	}
	return true;
}

s64 CStringLength(const char *s)
{
	auto len = 0;
	while (s[len])
	{
		len++;
	}
	return len;
}

bool IsCharWhitespace(char c)
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool IsCharDigit(char c)
{
	return '0' <= c && c <= '9';
}

char UppercaseChar(char c)
{
	if ('a' <= c && c <= 'z')
	{
		return 'A' + (c - 'a');
	}
	return c;
}

char LowercaseChar(char c)
{
	if ('A' <= c && c <= 'Z')
	{
		return 'a' + (c - 'A');
	}
	return c;
}

#if 0
StringView::StringView(const char *s, s64 len)
{
	this->buffer = NewArrayView<u8>((u8 *)s, len);
}

StringView::StringView(const char *s)
{
	auto len = CStringLength(s);
	this->buffer = NewArrayView<u8>((u8 *)s, len);
}

String StringView::Copy(s64 start, s64 end)
{
	Assert(end >= start);
	return
	{
		this->buffer.Copy(start, end),
		false,
	};
}

StringView StringView::View(s64 start, s64 end)
{
	return {this->buffer.View(start, end)};
}

StringView::StringView(ArrayView<u8> a)
	: buffer{a}
{
}

s64 StringView::Length()
{
	return this->buffer.count;
}

const char *StringView::CString()
{
	auto s = (u8 *)AllocateMemory(this->Length() + 1);
	CopyArray(this->buffer, NewArrayView(s, this->Length()));
	s[this->Length()] = '\0';
	return (const char *)s;
}
#endif

// @TODO: auto s = sb.String(); s = "1234"; ???
// @TODO: auto s = String{"1234"}; s = sb.String();
String::String()
{
	this->buffer = Array<u8>{};
	this->literal = true;
}

String::String(char *s)
{
	auto len = CStringLength(s);
	this->buffer = NewArray<u8>(len);
	CopyArray(NewArrayView((u8 *)s, len), this->buffer);
	this->literal = false;
}

String::String(const char *s)
{
	auto len = CStringLength(s);
	this->buffer = Array<u8>
	{
		.allocator = NullAllocator(),
		.elements = (u8 *)s,
		.count = len,
		.capacity = len,
	};
	this->literal = true;
}

String NewStringFromBuffer(Array<u8> b)
{
	String s;
	s.buffer = b;
	s.literal = false;
	return s;
}

#if 0
String::operator StringView()
{
	return {this->buffer};
}
#endif

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
	for (auto i = 0; i < s.Length(); i++)
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

const u8 *String::begin()
{
	return &(*this)[0];
}

const u8 *String::end()
{
	return &(*this)[this->Length()];
}

s64 String::Length()
{
	return this->buffer.count;
}

String String::Copy(s64 start, s64 end)
{
	Assert(end >= start);
	return NewStringFromBuffer(this->buffer.Copy(start, end));
}

char *String::CString()
{
	if (this->literal)
	{
		return (char *)this->buffer.elements;
	}
	auto s = (u8 *)AllocateMemory(this->Length() + 1);
	CopyArray(this->buffer, NewArrayView(s, this->Length()));
	s[this->Length()] = '\0';
	return (char *)s;
}

String String::View(s64 start, s64 end)
{
	Assert(start <= end);
	auto buf = Array<u8>
	{
		.allocator = NullAllocator(),
		.elements = &this->buffer[start],
		.count = end - start,
	};
	return NewStringFromBuffer(buf);
}

s64 String::FindFirst(u8 c)
{
	return this->buffer.FindFirst(c);
}

s64 String::FindLast(u8 c)
{
	return this->buffer.FindLast(c);
}

Array<String> String::Split(char seperator)
{
	auto splits = Array<String>{};
	auto curStart = 0;
	auto curLen = 0;
	for (auto i = 0; i < this->Length(); i++)
	{
		if ((*this)[i] == seperator)
		{
			if (curLen > 0)
			{
				splits.Append(this->Copy(curStart, i - 1));
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
		splits.Append(this->Copy(curStart, this->Length() - 1));
	}
	return splits;
}

// @TODO: Where did I get this from? Is the Knuth hash better?
u64 String::Hash()
{
    auto hash = u64{5381};
    for (auto c : this->buffer)
    {
        hash = ((hash << 5) + hash) + c;
	}
    return hash;
}

void String::Free()
{
	this->buffer.Free();
}

bool ParseInteger(String s, s64 *out)
{
    *out = 0;
    for (auto c : s)
    {
    	if (!IsCharDigit(c))
    	{
    		return false;
    	}
    	*out = (*out * 10) + c - '0';
    }
    return true;
}

StringBuilder NewStringBuilderIn(Allocator *a, s64 len)
{
	auto sb = StringBuilder
	{
		.buffer = NewArrayIn<u8>(a, len)
	};
	return sb;
}

StringBuilder NewStringBuilder(s64 len)
{
	return NewStringBuilderIn(ContextAllocator(), len);
}

StringBuilder NewStringBuilderWithCapacityIn(Allocator *a, s64 len, s64 cap)
{
	auto sb = StringBuilder
	{
		.buffer = NewArrayWithCapacityIn<u8>(a, len, cap)
	};
	return sb;
}

StringBuilder NewStringBuilderWithCapacity(s64 len, s64 cap)
{
	return NewStringBuilderWithCapacityIn(ContextAllocator(), len, cap);
}

#if 0
StringBuilder::operator StringView()
{
	return {this->buffer};
}
#endif

u8 &StringBuilder::operator[](s64 i)
{
	Assert(i >= 0 && i < this->buffer.count);
	return this->buffer[i];
}

const u8 &StringBuilder::operator[](s64 i) const
{
	Assert(i >= 0 && i < this->buffer.count);
	return this->buffer[i];
}

bool StringBuilder::operator==(StringBuilder sb)
{
	return this->buffer == sb.buffer;
}

bool StringBuilder::operator!=(StringBuilder sb)
{
	return this->buffer != sb.buffer;
}

u8 *StringBuilder::begin()
{
	return &(*this)[0];
}

u8 *StringBuilder::end()
{
	return &(*this)[this->Length()];
}

s64 StringBuilder::Length()
{
	return this->buffer.count;
}

String StringBuilder::String()
{
	return this->StringIn(ContextAllocator());
}

struct String StringBuilder::View(s64 start, s64 end)
{
	Assert(start <= end);
	auto buf = Array<u8>
	{
		.allocator = NullAllocator(),
		.elements = &this->buffer[start],
		.count = end - start,
	};
	return NewStringFromBuffer(buf);
}

struct String StringBuilder::StringIn(Allocator *a)
{
	auto buf = NewArrayIn<u8>(a, this->Length());
	CopyArray(this->buffer, buf);
	return NewStringFromBuffer(buf);
}

void StringBuilder::Resize(s64 len)
{
	this->buffer.Resize(len);
}

void StringBuilder::Append(struct String s)
{
	auto oldLen = this->Length();
	auto newLen = this->Length() + s.Length();
	this->Resize(newLen);
	CopyArray(s.buffer, this->buffer.View(oldLen, newLen));
}

void StringBuilder::AppendAll(ArrayView<struct String> ss)
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

void StringBuilder::Reserve(s64 reserve)
{
	this->buffer.Reserve(reserve);
}

void StringBuilder::Format(struct String fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	this->FormatVarArgs(fmt, args);
	va_end(args);
}

void StringBuilder::FormatVarArgs(struct String fmt, va_list args)
{
	this->Resize(this->Length() + STB_SPRINTF_MIN);
	auto Callback = [](char *, void *userData, s32 len) -> char *
	{
		auto sb = (StringBuilder *)userData;
		sb->buffer.Resize(sb->Length() + STB_SPRINTF_MIN);
		return (char *)&sb->buffer[len];
	};
	auto cfmt = fmt.CString();
	auto len = stbsp_vsprintfcb(Callback, this, (char *)&this->buffer[0], cfmt, args);
	DeallocateMemory(cfmt);
	this->buffer.Resize(len);
}

void StringBuilder::FormatTime()
{
	this->Append("%d-%d-%d %d:%d:%d.%d  ");
	// @TODO
}

void StringBuilder::Uppercase()
{
	for (auto i = 0; i < this->Length(); i++)
	{
		(*this)[i] = UppercaseChar((*this)[i]);
	}
}

void StringBuilder::Lowercase()
{
	for (auto i = 0; i < this->Length(); i++)
	{
		(*this)[i] = LowercaseChar((*this)[i]);
	}
}

s64 StringBuilder::FindFirst(u8 c)
{
	for (auto i = 0; i < this->Length(); i++)
	{
		if ((*this)[i] == c)
		{
			return i;
		}
	}
	return -1;
}

s64 StringBuilder::FindLast(u8 c)
{
	auto last = -1;
	for (auto i = 0; i < this->Length(); i++)
	{
		if ((*this)[i] == c)
		{
			last = i;
		}
	}
	return last;
}

void StringBuilder::Free()
{
	this->buffer.Free();
}
