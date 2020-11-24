#pragma once

#include "Basic/Parser.h"

namespace json
{

void EatFieldValue(parser::Parser *p);

template <typename F>
void ParseObject(parser::Parser *p, F &&proc)
{
	p->Expect('{');
	for (auto t = p->Token(); t != "" && t != "}"; t = p->Token())
	{
		auto name = t.ToView(1, t.Length() - 2);
		auto start = p->index;
		proc(p, name);
		if (p->index == start)
		{
			// User did not care about the value, so just eat it.
			EatFieldValue(p);
		}
		if (p->PeekChar() == ',')
		{
			p->Advance();
		}
	}
}

template <typename F>
void ParseList(parser::Parser *p, F &&proc)
{
	p->Expect('[');
	auto c = p->PeekChar();
	for (; c != -1 && c != ']'; c = p->PeekChar())
	{
		if (c == ',')
		{
			p->Advance();
		}
		auto start = p->index;
		proc(p);
		Assert(p->index != start);
	}
	if (c == ']')
	{
		p->Advance();
	}
}

}
