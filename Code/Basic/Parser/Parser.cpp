#include "Parser.h"
#include "../File.h"
#include "../Log.h"

namespace parser
{

Parser NewFromFile(string::String path, string::String delims, bool *err)
{
	auto buf = ReadEntireFile(path, err);
	if (*err)
	{
		return Parser{};
	}
	return
	{
		.string = string::NewFromBuffer(buf),
		.delimiters = delims,
		.line = 1,
		.column = 1,
	};
}

Parser NewFromString(string::String s, string::String delims)
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

string::String Parser::Line()
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

string::String Parser::Token()
{
	while (this->index < this->string.Length() && string::IsCharWhitespace(this->string[this->index]))
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
		while (this->index < this->string.Length() && !this->IsDelimiter(this->string[this->index]) && !string::IsCharWhitespace(this->string[this->index]))
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
	while (this->index < this->string.Length() && string::IsCharWhitespace(this->string[this->index]))
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
	while (this->index < this->string.Length() && string::IsCharWhitespace(this->string[this->index]))
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
