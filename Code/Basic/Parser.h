#pragma once

#include "String.h"

struct Parser
{
	String string;
	String delimiters;
	bool done;
	s64 index;
	s64 line;
	s64 column;

	bool IsDelimiter(char c);
	void Advance();
	String Line();
	String Token();
	void Eat(char c);
	s64 PeekChar();
	void Expect(u8 c);
};

Parser NewParser(String filepath, String delims, bool *err);
Parser NewParserFromString(String str, String delims);
