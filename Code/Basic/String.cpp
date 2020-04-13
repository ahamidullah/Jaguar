#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

char &String::operator[](size_t i)
{
	Assert(i < data.count);
	return data[i];
}

char &String::operator[](size_t i) const
{
	Assert(i < data.count);
	return data[i];
}

bool operator==(const String &a, const String &b)
{
	return a.data == b.data;
}

bool operator!=(const String &a, const String &b)
{
	return a.data != b.data;
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

String CreateString(size_t length, size_t capacity)
{
	String result =
	{
		.data = CreateArray<char>(length + 1, capacity + 1),
	};
	result.data[length] = '\0';
	return result;
}

String CreateStringCopy(const String &copy)
{
	auto length = Length(copy);
	String result = {
		.data = CreateArray<char>(length + 1),
	};
	CopyMemory(&copy[0], &result[0], length);
	result[length] = '\0';
	return result;
}

String CreateStringCopyRange(const String &copy, size_t startIndex, size_t endIndex)
{
	Assert(endIndex >= startIndex);
	auto length = endIndex - startIndex + 1;
	auto result = CreateString(length, length);
	CopyMemory(&copy[startIndex], &result[0], length);
	return result;
}

void ResizeString(String *string, size_t newSize)
{
	ResizeArray(&string->data, newSize + 1);
	string->data[newSize] = '\0';
}

void StringAppend(String *destination, const String &source)
{
	auto writeIndex = StringLength(destination);
	ResizeString(&destination->data, StringLength(destination) + StringLength(source) + 1);
	CopyMemory(&source.data[0], &destination->data[writeIndex], StringLength(source) + 1);
}

void StringAppendCString(String *destination, const char *source)
{
	auto sourceLength = CStringLength(source);
	auto writeIndex = StringLength(*destination);
	ResizeString(&destination->data, StringLength(destination) + sourceLength + 1);
	CopyMemory(source, &destination->data[writeIndex], sourceLength + 1);
}

void StringAppendRange(String *destination, const String &source, size_t rangeStartIndex, size_t rangeLength)
{
	Assert(source.data.count > rangeStartIndex);
	Assert(source.data.count >= rangeStartIndex + rangeLength);
	auto writeIndex = StringLength(*destination);
	ResizeString(&destination->data, Length(destination) + rangeLength + 1);
	CopyMemory(&source.data[rangeStartIndex], &destination->data[writeIndex], rangeLength + 1);
}

size_t StringLength(const String &s)
{
	if (ArrayCount(s.data) == 0)
	{
		return 0;
	}
	return ArrayCount(s.data) - 1; // To account for the NULL terminator we insert at the end.
}

size_t CStringLength(const char *string)
{
	auto length = 0;
	while (string[length])
	{
		length++;
	}
	return length;
}

String FormatStringVarArgs(const char *format, va_list arguments)
{
	auto result = CreateString(0);
	auto sprintfCallback = [&result](const char *buffer, void *userData, s32 length)
	{
		auto writeIndex = StringLength(result);
		ResizeString(&result, StringLength(result) + length);
		CopyMemory(buffer, &result[writeIndex], length);
	};
	char buffer[STB_SPRINTF_MIN];
	stbsp_vsprintf(sprintfCallback, NULL, buffer, format, arguments);
}

String FormatString(const String &format, ...)
{
	va_list arguments;
	va_start(arguments, format);
	auto result = FormatStringVarArgs(format, arguments);
	va_end(arguments);
	return result;
}

s64 FindFirstCharIndex(const String &s, char c)
{
	for (auto i = 0; i < StringLength(s); i++)
	{
		if (s.data[i] == c)
		{
			return i;
		}
	}
	return -1;
}

s64 FindLastCharIndex(const String &s, char c)
{
	s64 occurrence = -1;
	for (auto i = 0; i < StringLength(s); i++)
	{
		if (s.data[i] == c)
		{
			occurrence = i;
		}
	}
	return occurrence;
}

bool IsCharSpace(char c)
{
	if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
	{
		return true;
	}
	return false;
}

bool IsCharDigit(char c)
{
	if ('0' <= c && c <= '9')
	{
		return true;
	}
	return false;
}

void TrimString(String *s, size_t leftIndex, size_t rightIndex)
{
	Assert(rightIndex > leftIndex);
	auto length = rightIndex - (leftIndex + 1);
	MoveMemory(&s->data[leftIndex + 1], &s->data[0], length);
	ResizeString(s, length);
}

Array<String> SplitString(const String &s, char seperator)
{
	Array<String> result;
	auto splitStartIndex = 0;
	auto splitLength = 0;
	for (auto i = 0; i < StringLength(s); i++)
	{
		if (s[i] == seperator)
		{
			if (splitLength > 0)
			{
				Append(&result, CreateString(s, splitStartIndex, i - 1));
				splitLength = 0;
			}
		}
		else
		{
			if (splitLength == 0)
			{
				splitStartIndex = i;
			}
			splitLength++;
		}
	}
	if (splitLength > 0)
	{
		Append(&result, CreateString(s, splitStartIndex, Length(s) - 1));
	}
	return result;
}

bool ParseInteger(const String &s, s64 *result)
{
    *result = 0;
    for (auto c : s)
    {
    	if (!IsDigit(c))
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
	return &s[StringLength(*s)];
}

char *begin(String *s)
{
	return &(*s)[0];
}

char *end(String *s)
{
	return &(*s)[StringLength(*s)];
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
	for (auto i = 0; i < StringLength(*s); i++)
	{
		(*s)[i] = UppercaseChar((*s)[i]);
	}
}

void LowercaseString(String *s)
{
	for (auto i = 0; i < StringLength(*s); i++)
	{
		(*s)[i] = LowercaseChar((*s)[i]);
	}
}

u64 Hash(const String &s)
{
    u64 hash = 5381;
    for (auto c : s)
    {
        hash = ((hash << 5) + hash) + c;
	}
    return hash;
}
