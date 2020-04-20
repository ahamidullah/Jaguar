#pragma once

#define STANDARD_PARSER_DELIMITERS " \t:;\",(){}=-+*/\n"

struct ParserStream
{
	String string;
	String delimiters = STANDARD_PARSER_DELIMITERS;
	u32 index = 0;
	u32 lineCount = 1;
	u32 lineCharCount = 1;
};

ParserStream CreateParserStream(const String &filepath, bool *error, const String &delimiters = STANDARD_PARSER_DELIMITERS);
String GetToken(ParserStream *parser);
bool GetIfToken(ParserStream *parser, const String &expected);
bool GetUntilEndOfLine(ParserStream *parser, String *line);
bool ConsumeUntilChar(ParserStream *parser, char c);
