#include "JSON.h"

void JSONEatFieldValue(parser::Parser *p)
{
	auto t = p->Token();
	if (t == "{")
	{
		auto depth = 1;
		for (auto c = p->PeekChar(); c != -1 && depth > 0; c = p->PeekChar())
		{
			if (c == '}')
			{
				depth -= 1;
			}
			else if (c == '{')
			{
				depth += 1;
			}
			p->Advance();
		}
	}
	else if (t == "[")
	{
		auto depth = 1;
		for (auto c = p->PeekChar(); c != -1 && depth > 0; c = p->PeekChar())
		{
			if (c == ']')
			{
				depth -= 1;
			}
			else if (c == '[')
			{
				depth += 1;
			}
			p->Advance();
		}
	}
	else
	{
		// Eat the value, which can only be a string, a number, 'true', 'false', or 'null'.
	}
}
