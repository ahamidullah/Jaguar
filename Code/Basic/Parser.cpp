bool CreateParserStream(ParserStream *parser, const String &filepath)
{
	auto [string, error] = ReadEntireFile(filepath);
	if (error)
	{
		LogPrint(LogType::ERROR, "failed to create parser stream\n");
		return false;
	}
	parser->string = string;
	return true;
}

bool IsParserDelimiter(char c)
{
	const char *tokenDelimiters = " \t:;,(){}=-+\n";
	for (auto i = 0; i < Length(tokenDelimiters); i++)
	{
		if (c == tokenDelimiters[i])
		{
			return true;
		}
	}
	return false;
}

String GetParserToken(ParserStream *stream)
{
	while (stream->string[stream->index] && IsSpace(stream->string[stream->index]))
	{
		if (stream->string[stream->index] == '\n')
		{
			stream->line++;
		}
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

bool GetExpectedParserToken(ParserStream *stream, const String &expected)
{
	return expected == GetParserToken(stream);
}
