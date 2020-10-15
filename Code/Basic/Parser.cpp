#include "Parser.h"
#include "File.h"
#include "Log.h"

Parser NewParser(String filepath, String delims, bool *err)
{
	auto buf = ReadEntireFile(filepath, err);
	if (*err)
	{
		return Parser{};
	}
	return
	{
		.string = NewStringFromBuffer(buf),
		.delimiters = delims,
		.line = 1,
		.column = 1,
	};
}

Parser NewParserFromString(String str, String delims)
{
	return
	{
		.string = str,
		.delimiters = delims,
		.line = 1,
		.column = 1,
	};
}

bool Parser::IsDelimiter(char c)
{
	for (auto d : this->delimiters)
	{
		if (c == d)
		{
			return true;
		}
	}
	return false;
}

void Parser::Advance()
{
	if (this->index >= this->string.Length())
	{
		return;
	}
	if (this->string[this->index] == '\n')
	{
		this->line += 1;
		this->column = 0;
	}
	this->index += 1;
	this->column += 1;
}

String Parser::Line()
{
	if (this->index >= this->string.Length())
	{
		return "";
	}
	auto start = this->index;
	while (this->index < this->string.Length() && this->string[this->index] != '\n')
	{
		this->Advance();
	}
	if (this->string[this->index] == '\n')
	{
		this->Advance();
	}
	Assert(this->index > start);
	return this->string.View(start, this->index);
}

String Parser::Token()
{
	while (this->index < this->string.Length() && IsCharWhitespace(this->string[this->index]))
	{
		this->Advance();
	}
	if (this->index >= this->string.Length())
	{
		return "";
	}
	auto start = this->index;
	if (this->IsDelimiter(this->string[this->index]))
	{
		this->Advance();
	}
	else
	{
		while (this->index < this->string.Length() && !this->IsDelimiter(this->string[this->index]) && !IsCharWhitespace(this->string[this->index]))
		{
			this->Advance();
		}
	}
	Assert(this->index > start);
	return this->string.View(start, this->index);
}

void Parser::Eat(char c)
{
	while (this->index < this->string.Length() && this->string[this->index] != c)
	{
		this->Advance();
	}
	if (this->index < this->string.Length())
	{
		this->Advance();
	}
}

s64 Parser::PeekChar()
{
	while (this->index < this->string.Length() && IsCharWhitespace(this->string[this->index]))
	{
		this->Advance();
	}
	if (this->index >= this->string.Length())
	{
		return -1;
	}
	return this->string[this->index];
}

void Parser::Expect(u8 c)
{
	while (this->index < this->string.Length() && IsCharWhitespace(this->string[this->index]))
	{
		this->Advance();
	}
	if (this->index >= this->string.Length())
	{
		Abort("Parser", "Expected character %c, got end of parser.", (char)c);
	}
	if (this->string[this->index] != c)
	{
		Abort("Parser", "Expected character %c, got %c.", (char)c, (char)this->string[this->index]);
	}
	this->Advance();
}

#if 0
bool IsParserDelimiter(Parser *parser, char c)
{
	for (auto d : parser->delimiters)
	{
		if (c == d)
		{
			return true;
		}
	}
	return false;
}

void AdvanceParser(Parser *parser)
{
	if (parser->index >= StringLength(parser->string))
	{
		return;
	}
	if (parser->string[parser->index] == '\n')
	{
		parser->line++;
		parser->column = 0;
	}
	parser->index++;
	parser->column++;
}

String GetParserToken(Parser *parser)
{
	while (parser->index < StringLength(parser->string) && (parser->string[parser->index] == ' ' || parser->string[parser->index] == '\t'))
	{
		AdvanceParser(parser);
	}
	if (parser->index >= StringLength(parser->string))
	{
		return "";
	}
	auto tokenStartIndex = parser->index;
	if (IsParserDelimiter(parser, parser->string[parser->index]))
	{
		AdvanceParser(parser);
	}
	else
	{
		while (parser->index < StringLength(parser->string) && !IsParserDelimiter(parser, parser->string[parser->index]))
		{
			AdvanceParser(parser);
		}
	}
	Assert(parser->index > tokenStartIndex);
	return NewStringCopyRange(parser->string, tokenStartIndex, parser->index - 1);
}

String GetParserLine(Parser *parser)
{
	if (parser->index >= StringLength(parser->string))
	{
		return "";
	}
	auto lineStartIndex = parser->index;
	while (parser->index < StringLength(parser->string) && parser->string[parser->index] != '\n')
	{
		AdvanceParser(parser);
	}
	if (parser->string[parser->index] == '\n')
	{
		AdvanceParser(parser);
	}
	Assert(parser->index > lineStartIndex);
	return CreateString(parser->string, lineStartIndex, parser->index - 1);
}

/*
String ParserGetUntilChar(Parser *parser, char c)
{
	auto startIndex = parser->index;
	while (parser->string[parser->index] && parser->string[parser->index] != c)
	{
		AdvanceParser(parser);
	}
	if (!parser->string[parser->index] || startIndex == parser->index)
	{
		return "";
	}
	return CreateString(parser->string, startIndex, parser->index - 1);
}

bool GetIfParserToken(Parser *parser, const String &expected)
{
	auto initialParser = *parser;
	auto token = GetParserToken(parser);
	if (token == expected)
	{
		return true;
	}
	*parser = initialParser;
	return false;
}

String GetParserLine(Parser *parser)
{
	auto lineStartIndex = parser->index
	while (parser->string[parser->index] && parser->string[parser->index] != '\n')
	{
		AdvanceParser(parser);
	}
	if (parser->string[parser->index] != '\n')
	{
		AdvanceParser(parser);
	}
	return CreateString(parser->string, lineStartIndex, parser->index - 1);
}

String ParserGetUntilCharOrEnd(Parser *parser, char c)
{
	auto startIndex = parser->index;
	while (parser->string[parser->index] && parser->string[parser->index] != c)
	{
		AdvanceParser(parser);
	}
	return CreateString(parser->string, startIndex, parser->index - 1);
}

bool GetUntilEndOfLine(Parser *parser, String *line)
{
	ResizeString(line, 0);
	if (!parser->string[parser->index])
	{
		return false;
	}
	while (parser->string[parser->index] && parser->string[parser->index] != '\n')
	{
		StringAppend(line, parser->string[parser->index]);
		AdvanceParser(parser);
	}
	if (parser->string[parser->index] == '\n')
	{
		StringAppend(line, parser->string[parser->index]);
		AdvanceParser(parser);
	}
	return true;
}
*/
#endif
