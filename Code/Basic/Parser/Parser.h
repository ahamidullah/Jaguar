#pragma once

#include "Basic/String.h"

namespace parser
{

struct Parser
{
	string::String string;
	string::String delimiters;
	bool done;
	s64 index;
	s64 line;
	s64 column;

	bool IsDelimiter(char c);
	void Advance();
	string::String Line();
	string::String Token();
	void Eat(char c);
	s64 PeekChar();
	void Expect(u8 c);
};

Parser NewFromFile(string::String filepath, string::String delims, bool *err);
Parser NewFromString(string::String str, string::String delims);

}
