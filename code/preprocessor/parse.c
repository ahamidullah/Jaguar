// @TODO: Switch this to use a length string?

bool Is_Delimiter(char character) {
	const char *token_delimiters = " \t:;,(){}=-+\n";
	for (u32 i = 0; i < strlen(token_delimiters); i++) {
		if (character == token_delimiters[i]) {
			return true;
		}
	}
	return false;
}

typedef struct Stream {
	char *string;
} Stream;

typedef struct Token {
	char *string;
	u32 length;
} Token;

bool Token_Equals(Token token, const char *comparand) {
	u32 i = 0;
	for (; (i < token.length) && comparand[i]; i++) {
		if (token.string[i] != comparand[i]) {
			return false;
		}
	}
	if (i != token.length) {
		return false;
	}
	return true;
}

Token Do_Get_Token(Stream *stream) {
	while (*stream->string && isspace(*stream->string)) {
		stream->string++;
	}
	Token token = {
		.string = stream->string,
	};
	if (Is_Delimiter(*stream->string)) {
		stream->string++;
		token.length++;
		return token;
	}
	while (*stream->string) {
		stream->string++;
		token.length++;
	}
	return token;
}

Token Get_Token(Stream *stream) {
	Token token = Do_Get_Token(stream);
	if (token.length == 0) {
		Abort("Unexpected end of stream");
	}
	return token;
}

bool Try_To_Get_Token(Stream *stream, Token *token) {
	*token = Do_Get_Token(stream);
	if (token->length == 0) {
		return true;
	}
	return false;
}

void Get_Token_Into_Buffer(Stream *stream, char *buffer) {
	Token token = Get_Token(stream);
	for (u32 i = 0; i < token.length; i++) {
		buffer[i] = token.string[i];
	}
}

void Get_Expected_Token(Stream *stream, const char *expected) {
	Token token = Get_Token(stream);
	if (!Token_Equals(token, expected)) {
		Abort("Unexpected token: expected: %s, got: %s\n", expected, token); // @TODO
		//Abort("Unexpected token at %u:%u: expected: %s, got:'%s'.\n", line_number, character_offset, expected, token);
	}
}

// @TODO: Calling this a token isn't really right. Should be a string.
Token Get_Everything_In_Braces(Stream *stream) {
	Get_Expected_Token(stream, "{");
	s32 unmatched_brace_count = 1;
	Token result_token = {
		.string = stream->string,
	};
	while (unmatched_brace_count > 0) {
		Token token = Get_Token(stream);
		if (Token_Equals(token, "{")) {
			unmatched_brace_count++;
		}
		if (Token_Equals(token, "}")) {
			unmatched_brace_count--;
		}
		if (unmatched_brace_count > 0) {
			result_token.length++;
		}
	}
	return result_token;
}

char Peek_At_Next_Character(Stream *stream) {
	s32 i = 0;
	while (isspace(stream->string[i])) {
		i++;
	}
	return stream->string[i];
}

f32 Parse_Float_Value(Stream *stream) {
	Get_Expected_Token(stream, ":");
	char *new_stream_position;
	f32 result = strtof(stream->string, &new_stream_position);
	stream->string += (new_stream_position - stream->string);
	return result;
}

typedef struct V4 {
	f32 x, y, z, w;
} V4;

V4 Parse_Color_Value(Stream *stream) {
	V4 color;
	Get_Expected_Token(stream, ":");
	Get_Expected_Token(stream, "{");
	Token token;
	while ((token = Get_Token(stream)), !Token_Equals(token, "}")) {
		if (Token_Equals(token, "r")) {
			color.x = Parse_Float_Value(stream);
		} else if (Token_Equals(token, "g")) {
			color.y = Parse_Float_Value(stream);
		} else if (Token_Equals(token, "b")) {
			color.z = Parse_Float_Value(stream);
		} else if (Token_Equals(token, "a")) {
			color.w = Parse_Float_Value(stream);
		} else {
			Abort("Unexpected token %s while parsing color", token);
		}
		Get_Expected_Token(stream, ",");
	}
	return color;
}

Token Parse_String_Value(Stream *stream) {
	Get_Expected_Token(stream, ":");
	return Get_Token(stream);
}
