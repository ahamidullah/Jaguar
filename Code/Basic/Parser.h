#pragma once

struct ParserStream
{
	String string;
	u32 index = 0;
};

bool CreateParserStreamFromFile(ParserStream *parser, const String &filepath);
String GetToken(ParserStream *stream);
bool GetExpectedToken(ParserStream *stream, const String &expected);
char PeekAtNextCharacter(ParserStream *parser);
void AdvanceParser(ParserStream *parser, u32 count);
bool GetLine(ParserStream *parser, String *line);
