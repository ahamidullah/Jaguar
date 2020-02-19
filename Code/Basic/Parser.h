#pragma once

struct ParserStream
{
	String string;
	u32 index = 0;
	u32 line = 0;
};

bool CreateParserStream(ParserStream *parser, const String &filepath);
String GetParserToken(ParserStream *stream);
bool GetExpectedParserToken(ParserStream *stream, const char *expected);
