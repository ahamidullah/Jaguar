#include "String.h"
#define STB_SPRINTF_IMPLEMENTATION
	#include "stb_sprintf.h"
#undef STB_SPRINTF_IMPLEMENTATION

char &String::operator[](s64 i)
{
	Assert(i >= 0 && i < data.count);
	return data[i];
}

char &String::operator[](s64 i) const
{
	Assert(i >= 0 && i < data.count);
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

String CreateString(s64 length)
{
	auto result = String{CreateArray<char>(length + 1)};
	result.data[length] = '\0';
	return result;
}

String CreateString(s64 length, s64 capacity)
{
	auto result = String{CreateArray<char>(length + 1, capacity + 1)};
	result.data[length] = '\0';
	return result;
}

String CreateString(const String &copy)
{
	auto length = StringLength(copy);
	auto result = String{CreateArray<char>(length + 1)};
	CopyMemory(&copy[0], &result[0], length);
	result[length] = '\0';
	return result;
}

String CreateString(const String &copy, s64 startIndex, s64 endIndex)
{
	Assert(endIndex >= startIndex);
	auto length = endIndex - startIndex + 1;
	auto result = CreateString(length);
	CopyMemory(&copy[startIndex], &result[0], length);
	return result;
}

s64 StringLength(const String &s)
{
	if (ArrayLength(s.data) == 0)
	{
		return 0;
	}
	return ArrayLength(s.data) - 1; // To account for the NULL terminator we insert at the end.
}

void ResizeString(String *string, s64 newSize)
{
	ResizeArray(&string->data, newSize + 1);
	string->data[newSize] = '\0';
}

void StringAppend(String *destination, const String &source)
{
	auto writeIndex = StringLength(*destination);
	ResizeArray(&destination->data, StringLength(*destination) + StringLength(source) + 1);
	CopyMemory(&source.data[0], &destination->data[writeIndex], StringLength(source) + 1);
}

void StringAppend(String *destination, const char *source)
{
	auto sourceLength = CStringLength(source);
	auto writeIndex = StringLength(*destination);
	ResizeArray(&destination->data, StringLength(*destination) + sourceLength + 1);
	CopyMemory(source, &destination->data[writeIndex], sourceLength + 1);
}

void StringAppend(String *destination, const String &source, s64 rangeStartIndex, s64 rangeLength)
{
	Assert(source.data.count > rangeStartIndex);
	Assert(source.data.count >= rangeStartIndex + rangeLength);
	auto writeIndex = StringLength(*destination);
	ResizeArray(&destination->data, StringLength(*destination) + rangeLength + 1);
	CopyMemory(&source.data[rangeStartIndex], &destination->data[writeIndex], rangeLength + 1);
}

void StringAppend(String *destination, char source)
{
	ResizeArray(&destination->data, StringLength(*destination) + 2);
	(*destination)[StringLength(*destination) - 1] = source;
}

String JoinStrings(const String &a, const String &b)
{
	auto result = CreateString(StringLength(a) + StringLength(b));
	CopyMemory(&a[0], &result[0], StringLength(a));
	CopyMemory(&b[0], &result[StringLength(a)], StringLength(b));
	return result;
}

s64 CStringLength(const char *string)
{
	auto length = 0;
	while (string[length])
	{
		length++;
	}
	return length;
}

void SetMinimumStringCapacity(String *string, s64 minimumCapacity)
{
	SetMinimumArrayCapacity(&string->data, minimumCapacity + 1);
}

String FormatStringVarArgs(const String &format, va_list arguments)
{
	auto stringBuffer = CreateArray<char>(STB_SPRINTF_MIN);
	struct SprintfCallbackData
	{
		Array<char> *stringBuffer;
		s64 stringLength;
	} sprintfCallbackData =
	{
		.stringBuffer = &stringBuffer,
		.stringLength = 0,
	};
	auto SprintfCallback = [](char *buffer, void *userData, s32 length)
	{
		auto data = (SprintfCallbackData *)userData;
		data->stringLength += length;
		ResizeArray(data->stringBuffer, data->stringLength + STB_SPRINTF_MIN);
		return &(*data->stringBuffer)[data->stringLength];
	};
	stbsp_vsprintfcb(SprintfCallback, &sprintfCallbackData, &stringBuffer[0], &format[0], arguments);
	stringBuffer[sprintfCallbackData.stringLength] = '\0';
	ResizeArray(&stringBuffer, sprintfCallbackData.stringLength + 1);
	return String{stringBuffer};
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

bool IsCharWhitespace(char c)
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool IsCharDigit(char c)
{
	return '0' <= c && c <= '9';
}

void TrimString(String *s, s64 leftIndex, s64 rightIndex)
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
				ArrayAppend(&result, CreateString(s, splitStartIndex, i - 1));
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
		ArrayAppend(&result, CreateString(s, splitStartIndex, StringLength(s) - 1));
	}
	return result;
}

bool ParseInteger(const String &s, s64 *result)
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
	return &s[StringLength(s)];
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

void ArrayAppend(Array<String> *a, const char *newElement)
{
	ArrayAppend<String>(a, String{newElement});
}
