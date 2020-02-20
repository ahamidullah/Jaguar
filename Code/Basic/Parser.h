#pragma once

struct ParserStream
{
	String string;
	u32 index = 0;
	u32 line = 0;
};

bool CreateParserStream(ParserStream *parser, const String &filepath);
String GetToken(ParserStream *stream);
bool GetExpectedToken(ParserStream *stream, const char *expected);
char PeekAtNextCharacter(ParserStream *parser);
void AdvanceParser(ParserStream *parser, u32 count);
