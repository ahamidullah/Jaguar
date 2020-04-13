bool CreateParserStreamFromFile(ParserStream *parser, const String &filepath)
{
	auto [string, error] = ReadEntireFile(filepath);
	if (error)
	{
		return false;
	}
	parser->string = string;
	return true;
}

bool IsParserDelimiter(char c)
{
	const char *tokenDelimiters = " \t:;\",(){}=-+*/\n";
	for (auto i = 0; i < Length(tokenDelimiters); i++)
	{
		if (c == tokenDelimiters[i])
		{
			return true;
		}
	}
	return false;
}

String GetToken(ParserStream *stream)
{
	while (stream->string[stream->index] && (stream->string[stream->index] == ' ' || stream->string[stream->index] == '\t'))
	{
		stream->index++;
	}
	if (!stream->string[stream->index])
	{
		return CreateString(0);
	}
	auto tokenStartIndex = stream->index;
	if (IsParserDelimiter(stream->string[stream->index]))
	{
		stream->index++;
	}
	else
	{
		while (stream->string[stream->index] && !IsParserDelimiter(stream->string[stream->index]))
		{
			stream->index++;
		}
	}
	Assert(stream->index > tokenStartIndex);
	return CreateString(stream->string, tokenStartIndex, stream->index - 1);
}

bool GetExpectedToken(ParserStream *parser, const String &expected)
{
	return expected == GetToken(parser);
}

bool GetLine(ParserStream *parser, String *line)
{
	Resize(line, 0);
	if (!parser->string[parser->index])
	{
		return false;
	}
	while (parser->string[parser->index] && parser->string[parser->index] != '\n')
	{
		Append(line, parser->string[parser->index]);
		parser->index++;
	}
	if (parser->string[parser->index] == '\n')
	{
		Append(line, parser->string[parser->index]);
		parser->index++;
	}
	return true;
}

char PeekAtNextCharacter(ParserStream *parser)
{
	return parser->string[parser->index];
}

void AdvanceParser(ParserStream *parser, u32 count)
{
	for (auto i = 0; i < count && parser->string[parser->index]; i++)
	{
		parser->index++;
	}
}
