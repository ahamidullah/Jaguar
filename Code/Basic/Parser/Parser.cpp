#include "Parser.h"
#include "../File.h"
#include "../Log.h"

namespace parser
{

Parser File(String path, String delims, bool *err)
{
	auto buf = ReadEntireFile(path, err);
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

Parser XString(String s, String delims)
{
	return
	{
		.string = s,
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
	return this->string.ToView(start, this->index);
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
	return this->string.ToView(start, this->index);
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

}
