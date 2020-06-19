#pragma once

#include "String.h"

#define STANDARD_PARSER_DELIMITERS " \t:;\",(){}=-+*/\n"

struct Parser
{
	String string;
	String delimiters;
	s64 index = 0;
	s64 line = 1;
	s64 column = 1;
};

Parser CreateParser(const String &filepath, const String &delimiters, bool *error);
String GetParserToken(Parser *parser);
String GetParserLine(Parser *parser);
