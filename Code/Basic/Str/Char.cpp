#include "Char.h"

namespace str
{

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

}
