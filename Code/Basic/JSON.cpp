#if 0
#include "JSON.h"
#include "Parser.h"

s64 JSONValue::Number()
{
	Assert(this->type == JSONNumberType);
	return this->number;
}

struct String JSONValue::String()
{
	Assert(this->type == JSOStringType);
	return this->string;
}

bool JSONValue::Boolean()
{
	Assert(this->type == JSONBooleanType);
	return this->boolean;
}

Array<JSONValue> JSONValue::Array()
{
	Assert(this->type == JSONArrayType);
	return this->array;
}

JSONObject JSONValue::Object()
{
	Assert(this->type == JSONObjectType);
	return this->object;
}

s64 JSONParseNumber(String t, bool *err)
{
}

JSONValue JSONParseValue(Parser *p, bool *err)
{
	auto v = JSONValue{};
	auto t = p->Token();
	if (t == "\"")
	{
		v.type = JSONStringType;
		v.string = p->Token();
		if (auto t = p->Token(); t != "\"")
		{
			LogError("JSON", "Line: %ld, column: %ld: failed parsing string, expected '\"', got %k.", p->line, p->column, t);
			*err = true;
			return {};
		}
	}
	else if (t.Length() > 0 && IsCharDigit(t[0]))
	{
		v.type = JSONNumberType;
		v.number = ParseInteger(t, err);
		if (*err)
		{
			LogError("JSON", "Line: %ld, column: %ld: failed parsing number, non-digit character in %k.", p->line, p->column, t);
			return 0;
		}
	}
	else if (t == "true")
	{
		v.type = JSONBooleanType
		v.boolean = true;
	}
	else if (t == "false")
	{
		v.type = JSONBooleanType
		v.boolean = false;
	}
	else if (t == "{")
	{
		v.type = JSONObjectType;
		for (; t != "}"; t = p->Token())
		{
			auto pair = JSONObjectField{};
			pair.name = p->Token();
		}
		v.object = JSONParseObject(p, err);
	}
	else if (t == "[")
	{
		v.type = JSONArrayType;
		v.array = JSONParseArray(p, err);
	}
	else
	{
		LogError("JSON", "Line: %ld, column: %ld: failed parsing value, expected one of String, Number, 'null', 'true', 'false', '{', or '[', got %k.", p->line, p->column, t);
		*err = true;
		return {};
	}
	return v;
}

JSONValue JSONParseFile(String path, bool *err)
{
	auto p = NewParser(path, ":[],{}\"");
	auto j = JSONValue{};
	return JSONParseValue(&p, err);
}
#endif
