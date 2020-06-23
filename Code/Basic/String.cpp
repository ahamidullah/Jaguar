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
	auto length = 0;
	while (s[length])
	{
		length++;
	}
	return length;
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

String::String()
{
	this->buffer = "";
	this->length = 0;
	this->literal = true;
}

String::String(char *s)
{
	this->buffer = s;
	this->length = CStringLength(s);
	this->literal = false;
}

String::String(char *s, s64 len)
{
	this->buffer = s;
	this->length = len;
	this->literal = false;
}

String::String(const char *s, s64 len)
{
	this->buffer = s;
	this->length = len;
	this->literal = true;
}

String::String(const char *s)
{
	this->buffer = s;
	this->length = CStringLength(s);
	this->literal = true;
}

String::String(AllocatorInterface a, const char *b, s64 len, bool lit)
{
	this->allocator = a;
	this->buffer = b;
	this->length = len;
	this->literal = lit;
}

const char &String::operator[](s64 i) const
{
	Assert(i >= 0 && i < length);
	return buffer[i];
}

bool operator==(String a, String b)
{
	if (a.length != b.length)
	{
		return false;
	}
	for (auto i = 0; i < a.length; i++)
	{
		if (a.buffer[i] != b.buffer[i])
		{
			return false;
		}
	}
	return true;
}

bool operator!=(String a, String b)
{
	return !(a == b);
}

const char *begin(String s)
{
	return &s[0];
}

const char *end(String s)
{
	return &s[s.length];
}

const char *begin(String *s)
{
	return &(*s)[0];
}

const char *end(String *s)
{
	return &(*s)[s->length];
}

String NewStringCopy(String src)
{
	auto buf = (char *)AllocateMemory(src.length + 1);
	CopyMemory(src.buffer, buf, src.length);
	buf[src.length] = '\0';
	return
	{
		ContextAllocator(),
		buf,
		src.length,
		false,
	};
}

String NewStringCopyRange(String src, s64 start, s64 end)
{
	Assert(end >= start);
	auto len = end - start;
	auto buf = (char *)AllocateMemory(len + 1);
	CopyMemory(&src.buffer[start], buf, len);
	buf[len] = '\0';
	return
	{
		ContextAllocator(),
		buf,
		len,
		false,
	};
}

s64 FindFirstCharInString(String s, char c)
{
	for (auto i = 0; i < s.length; i++)
	{
		if (s[i] == c)
		{
			return i;
		}
	}
	return -1;
}

s64 FindLastCharInString(String s, char c)
{
	auto last = -1;
	for (auto i = 0; i < s.length; i++)
	{
		if (s[i] == c)
		{
			last = i;
		}
	}
	return last;
}

Array<String> SplitString(String s, char seperator)
{
	auto splits = Array<String>{};
	auto curStart = 0;
	auto curLen = 0;
	for (auto i = 0; i < s.length; i++)
	{
		if (s[i] == seperator)
		{
			if (curLen > 0)
			{
				AppendToArray(&splits, NewStringCopyRange(s, curStart, i - 1));
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
		AppendToArray(&splits, NewStringCopyRange(s, curStart, s.length - 1));
	}
	return splits;
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

// @TODO: Where did I get this from? Is the Knuth hash better?
u64 HashString(String s)
{
    auto hash = u64{5381};
    for (auto c : s)
    {
        hash = ((hash << 5) + hash) + c;
	}
    return hash;
}

void FreeString(String *s)
{
	if (!s->literal)
	{
		s->allocator.freeMemory(s->allocator.data, (char *)s->buffer);
	}
}

// This overload allows us to append a c-string to an Array of Strings without explicitly constructing a String.
void AppendToArray(Array<String> *a, const char *s)
{
	AppendToArray<String>(a, String{s});
}

char &StringBuilder::operator[](s64 i)
{
	Assert(i >= 0 && i < buffer.count);
	return buffer[i];
}

const char &StringBuilder::operator[](s64 i) const
{
	Assert(i >= 0 && i < buffer.count);
	return buffer[i];
}

bool operator==(StringBuilder a, StringBuilder b)
{
	return a.buffer == b.buffer;
}

bool operator!=(StringBuilder a, StringBuilder b)
{
	return a.buffer != b.buffer;
}

char *begin(StringBuilder s)
{
	return &s[0];
}

char *end(StringBuilder s)
{
	return &s[s.length];
}

char *begin(StringBuilder *s)
{
	return &(*s)[0];
}

char *end(StringBuilder *s)
{
	return &(*s)[s->length];
}

StringBuilder NewStringBuilder(s64 len)
{
	auto sb = StringBuilder{NewArray<char>(len + 1)};
	sb[len] = '\0';
	sb.length = len;
	return sb;
}

StringBuilder NewStringBuilderIn(s64 len, AllocatorInterface a)
{
	auto sb = StringBuilder{NewArrayIn<char>(len + 1, a)};
	sb[len] = '\0';
	sb.length = len;
	return sb;
}

StringBuilder NewStringBuilderWithCapacity(s64 len, s64 cap)
{
	auto sb = StringBuilder{NewArrayWithCapacity<char>(len + 1, cap + 1)};
	sb[len] = '\0';
	sb.length = len;
	return sb;
}

StringBuilder NewStringBuilderWithCapacityIn(s64 len, s64 cap, AllocatorInterface a)
{
	auto sb = StringBuilder{NewArrayWithCapacityIn<char>(len + 1, cap + 1, a)};
	sb[len] = '\0';
	sb.length = len;
	return sb;
}

#if 0
String NewStringCopy(String s)
{
	auto r = NewString(s.length);
	CopyMemory(&s[0], &r[0], s.length);
	return r;
}

String NewStringCopyIn(String s, AllocatorInterface a)
{
	auto r = NewStringIn(s.length, a);
	CopyMemory(&s[0], &r[0], s.length);
	return r;
}

String NewStringCopyRange(String s, s64 start, s64 end)
{
	Assert(end >= start);
	auto r = NewString(end - start + 1);
	CopyMemory(&s[start], &r[0], result.length);
	return r;
}

String NewStringCopyRangeIn(String s, s64 start, s64 end, AllocatorInterface a)
{
	Assert(end >= start);
	auto r = NewStringIn(end - start + 1, a);
	CopyMemory(&s[start], &r[0], r.length);
	return r;
}
#endif

void FreeStringBuilder(StringBuilder *sb)
{
	FreeArray(&sb->buffer);
	sb->length = 0;
}

String BuilderToString(StringBuilder sb)
{
	auto buf = (char *)AllocateMemory(sb.buffer.count);
	CopyMemory(&sb.buffer[0], buf, sb.buffer.count);
	return
	{
		ContextAllocator(),
		buf,
		sb.length,
		false,
	};
}

String BuilderToStringIn(StringBuilder *sb, AllocatorInterface a)
{
	auto buf = (char *)a.allocateMemory(a.data, sb->buffer.count);
	CopyMemory(&sb->buffer[0], buf, sb->buffer.count);
	return
	{
		a,
		buf,
		sb->length,
		false,
	};
}

void ResizeStringBuilder(StringBuilder *sb, s64 len)
{
	ResizeArray(&sb->buffer, len + 1);
	sb->length = len;
	(*sb)[len] = '\0';
}

void AppendToStringBuilder(StringBuilder *dst, String src)
{
	auto i = dst->length;
	ResizeStringBuilder(dst, dst->length + src.length);
	CopyMemory(&src[0], &(*dst)[i], src.length);
}

#if 0
void StringBuilderAppendData(String *dst, const char *src)
{
	auto n = CStringLength(src);
	auto i = dst->length;
	ResizeString(dst, dst->length + n);
	CopyMemory(src, &(*dst)[i], n);
}
#endif

void AppendRangeToStringBuilder(StringBuilder *dst, String src, s64 start, s64 end)
{
	Assert(end >= start);
	Assert(src.length >= end);
	auto n = (end - start);
	auto i = dst->length;
	ResizeStringBuilder(dst, dst->length + n);
	CopyMemory(&src[start], &(*dst)[i], n);
}

#if 0
void AppendCharToString(String *d, char s)
{
	ResizeString(d, d->length + 1);
	(*d)[d->length - 1] = s;
}
#endif

void ReserveStringBuilder(StringBuilder *sb, s64 reserve)
{
	ReserveArray(&sb->buffer, reserve + 1);
}

void FormatString(StringBuilder *sb, String fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	FormatStringVarArgs(sb, fmt, args);
	va_end(args);
}

void FormatStringVarArgs(StringBuilder *sb, String fmt, va_list args)
{
	#if 0
	struct CallbackData
	{
		StringBuilder *sb
	} cb =
	{
		.sb = sb,
	};
	#endif
	ResizeStringBuilder(sb, sb->length + STB_SPRINTF_MIN);
	auto Callback = [](char *, void *userData, s32 len)
	{
		auto csb = (StringBuilder *)userData;
		csb->length += len;
		ResizeArray(&csb->buffer, csb->length + STB_SPRINTF_MIN);
		return &csb->buffer[csb->length];
	};
	stbsp_vsprintfcb(Callback, sb, &sb->buffer[0], &fmt[0], args);
	sb->buffer[sb->length] = '\0';
	ResizeArray(&sb->buffer, sb->length + 1);
}

#if 0
@TODO: WRONG?
void TrimStringBuilder(StringBuilder *sb, s64 left, s64 right)
{
	Assert(right > left);
	auto n = right - (left + 1);
	MoveMemory(&sb->buffer[left + 1], &sb->buffer[0], length);
	StringBuilderResize(sb, n);
}
#endif

void UppercaseStringBuilder(StringBuilder *sb)
{
	for (auto i = 0; i < sb->length; i++)
	{
		(*sb)[i] = UppercaseChar((*sb)[i]);
	}
}

void LowercaseStringBuilder(StringBuilder *sb)
{
	for (auto i = 0; i < sb->length; i++)
	{
		(*sb)[i] = LowercaseChar((*sb)[i]);
	}
}

s64 FindFirstCharInStringBuilder(StringBuilder sb, char c)
{
	for (auto i = 0; i < sb.length; i++)
	{
		if (sb[i] == c)
		{
			return i;
		}
	}
	return -1;
}

s64 FindLastCharInStringBuilder(StringBuilder sb, char c)
{
	auto last = -1;
	for (auto i = 0; i < sb.length; i++)
	{
		if (sb[i] == c)
		{
			last = i;
		}
	}
	return last;
}

