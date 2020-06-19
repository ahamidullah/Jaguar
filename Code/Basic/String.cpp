#include "Basic.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"
#undef STB_SPRINTF_IMPLEMENTATION

char &String::operator[](s64 i)
{
	Assert(i >= 0 && i < buffer.count);
	return buffer[i];
}

char &String::operator[](s64 i) const
{
	Assert(i >= 0 && i < buffer.count);
	return buffer[i];
}

bool operator==(const String &a, const String &b)
{
	return a.buffer == b.buffer;
}

bool operator!=(const String &a, const String &b)
{
	return a.buffer != b.buffer;
}

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

String NewString(s64 length)
{
	auto result = String{NewArray<char>(length + 1)};
	result[length] = '\0';
	result.length = length;
	return result;
}

String NewStringIn(s64 length, AllocatorInterface a)
{
	auto result = String{NewArrayIn<char>(length + 1, a)};
	result[length] = '\0';
	result.length = length;
	return result;
}

String NewStringWithCapacity(s64 length, s64 capacity)
{
	auto result = String{NewArrayWithCapacity<char>(length + 1, capacity + 1)};
	result[length] = '\0';
	result.length = length;
	return result;
}

String NewStringWithCapacityIn(s64 length, s64 capacity, AllocatorInterface a)
{
	auto result = String{NewArrayWithCapacityIn<char>(length + 1, capacity + 1, a)};
	result[length] = '\0';
	result.length = length;
	return result;
}

String NewStringCopy(String s)
{
	auto result = NewString(s.length);
	CopyMemory(&s[0], &result[0], s.length);
	return result;
}

String NewStringCopyIn(String s, AllocatorInterface a)
{
	auto result = NewStringIn(s.length, a);
	CopyMemory(&s[0], &result[0], s.length);
	return result;
}

String NewStringCopyRange(String s, s64 start, s64 end)
{
	Assert(end >= start);
	auto result = NewString(end - start + 1);
	CopyMemory(&s[start], &result[0], result.length);
	return result;
}

String NewStringCopyRangeIn(String s, s64 start, s64 end, AllocatorInterface a)
{
	Assert(end >= start);
	auto result = NewStringIn(end - start + 1, a);
	CopyMemory(&s[start], &result[0], result.length);
	return result;
}

void ResizeString(String *s, s64 length)
{
	ResizeArray(&s->buffer, s->buffer.count);
	s->length = length;
	(*s)[length] = '\0';
}

void AppendToString(String *d, String s)
{
	auto i = d->length;
	ResizeString(d, d->length + s.length);
	CopyMemory(&s[0], &(*d)[i], s.length);
}

void AppendDataToString(String *d, const char *s)
{
	auto sLength = CStringLength(s);
	auto i = d->length;
	ResizeString(d, d->length + sLength);
	CopyMemory(s, &(*d)[i], sLength);
}

void AppendRangeToString(String *d, String s, s64 start, s64 end)
{
	Assert(end >= start);
	Assert(s.length >= end);
	auto length = (end - start);
	auto i = d->length;
	ResizeString(d, d->length + length);
	CopyMemory(&s[start], &(*d)[i], length);
}

void AppendCharToString(String *d, char s)
{
	ResizeString(d, d->length + 1);
	(*d)[d->length - 1] = s;
}

String JoinStrings(String a, String b)
{
	auto result = NewString(a.length + b.length);
	CopyMemory(&a[0], &result[0], a.length);
	CopyMemory(&b[0], &result[a.length], b.length);
	return result;
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

void ReserveString(String *s, s64 reserve)
{
	ReserveArray(&s->buffer, reserve + 1);
}

String FormatStringVarArgs(String format, va_list arguments)
{
	auto buffer = NewArray<char>(STB_SPRINTF_MIN);
	struct CallbackData
	{
		Array<char> *buffer;
		s64 length;
	} callbackData =
	{
		.buffer = &buffer,
		.length = 0,
	};
	auto callback = [](char *, void *userData, s32 length)
	{
		auto data = (CallbackData *)userData;
		data->length += length;
		ResizeArray(data->buffer, data->length + STB_SPRINTF_MIN);
		return &(*data->buffer)[data->length];
	};
	stbsp_vsprintfcb(callback, &callbackData, &buffer[0], &format[0], arguments);
	buffer[callbackData.length] = '\0';
	ResizeArray(&buffer, callbackData.length + 1);
	return String{buffer};
}

String FormatString(String format, ...)
{
	va_list arguments;
	va_start(arguments, format);
	auto result = FormatStringVarArgs(format, arguments);
	va_end(arguments);
	return result;
}

s64 FindFirstChar(String s, char c)
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

s64 FindLastChar(String s, char c)
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

bool IsCharWhitespace(char c)
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool IsCharDigit(char c)
{
	return '0' <= c && c <= '9';
}

void TrimString(String *s, s64 left, s64 right)
{
	Assert(right > left);
	auto length = right - (left + 1);
	MoveMemory(&s->buffer[left + 1], &s->buffer[0], length);
	ResizeString(s, length);
}

Array<String> SplitString(String s, char seperator)
{
	auto result = Array<String>{};
	auto splitStart = 0;
	auto splitLength = 0;
	for (auto i = 0; i < s.length; i++)
	{
		if (s[i] == seperator)
		{
			if (splitLength > 0)
			{
				AppendToArray(&result, NewStringCopyRange(s, splitStart, i - 1));
				splitLength = 0;
			}
		}
		else
		{
			if (splitLength == 0)
			{
				splitStart = i;
			}
			splitLength += 1;;
		}
	}
	if (splitLength > 0)
	{
		AppendToArray(&result, NewStringCopyRange(s, splitStart, s.length - 1));
	}
	return result;
}

bool ParseInteger(String s, s64 *result)
{
    *result = 0;
    for (auto c : s)
    {
    	if (!IsCharDigit(c))
    	{
    		return false;
    	}
    	*result = (*result * 10) + c - '0';
    }
    return true;
}

char *begin(const String &s)
{
	return &s[0];
}

char *end(const String &s)
{
	return &s[s.length];
}

char *begin(String *s)
{
	return &(*s)[0];
}

char *end(String *s)
{
	return &(*s)[s->length];
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

void UppercaseString(String *s)
{
	for (auto i = 0; i < s->length; i++)
	{
		(*s)[i] = UppercaseChar((*s)[i]);
	}
}

void LowercaseString(String *s)
{
	for (auto i = 0; i < s->length; i++)
	{
		(*s)[i] = LowercaseChar((*s)[i]);
	}
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

// This overload allows us to append a c-string to an Array of Strings without explicitly constructing a String.
void AppendToArray(Array<String> *a, const char *newElement)
{
	AppendToArray<String>(a, String{newElement});
}
