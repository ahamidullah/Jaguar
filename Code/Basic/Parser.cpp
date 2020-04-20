ParserStream CreateParserStream(const String &filepath, const String &delimiters, bool *error)
{
	auto fileString = ReadEntireFile(filepath, error);
	if (*error)
	{
		return ParserStream{};
	}
	*error = false;
	return
	{
		.string = fileString,
		.delimiters = delimiters,
		.index = 0,
	};
}

bool IsParserDelimiter(ParserStream *parser, char c)
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

void AdvanceParser(ParserStream *parser)
{
	if (parser->string[parser->index] == '\n')
	{
		parser->lineCount++;
		parser->lineCharCount = 0;
	}
	parser->index++;
	parser->lineCharCount++;
}

String GetToken(ParserStream *parser)
{
	while (parser->string[parser->index] && (parser->string[parser->index] == ' ' || parser->string[parser->index] == '\t'))
	{
		AdvanceParser(parser);
	}
	if (!parser->string[parser->index])
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
		while (parser->string[parser->index] && !IsParserDelimiter(parser, parser->string[parser->index]))
		{
			AdvanceParser(parser);
		}
	}
	Assert(parser->index > tokenStartIndex);
	return CreateString(parser->string, tokenStartIndex, parser->index - 1);
}

bool ConsumeUntilChar(ParserStream *parser, char c)
{
	while (parser->string[parser->index] && parser->string[parser->index] != c)
	{
		AdvanceParser(parser);
	}
	if (parser->string[parser->index] != c)
	{
		return false;
	}
	return true;
}

bool GetIfToken(ParserStream *parser, const String &expected)
{
	auto initialParser = *parser;
	auto token = GetToken(parser);
	if (token == expected)
	{
		return true;
	}
	*parser = initialParser;
	return false;
}

bool GetUntilEndOfLine(ParserStream *parser, String *line)
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
