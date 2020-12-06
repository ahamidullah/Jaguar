#pragma once

#include "Basic/String.h"

namespace parser
{

struct Parser
{
	str::String string;
	str::String delimiters;
	bool done;
	s64 index;
	s64 line;
	s64 column;

	bool IsDelimiter(char c);
	void Advance();
	str::String Line();
	str::String Token();
	void Eat(char c);
	s64 PeekChar();
	void Expect(u8 c);
};

Parser NewFromFile(str::String filepath, str::String delims, bool *err);
Parser NewFromString(str::String str, str::String delims);

}
