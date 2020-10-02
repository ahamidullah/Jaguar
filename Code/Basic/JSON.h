#pragma once

#include "Basic/Array.h"

struct JSONObjectField;

struct JSONObject
{
	Array<JSONObjectField> pairs;
};

enum JSONValueType
{
	JSONNumberType,
	JSONStringType,
	JSONBooleanType,
	JSONArrayType,
	JSONObjectType,
};

struct JSONValue
{
	JSONValueType type;
	union
	{
		s64 number;
		String string;
		bool boolean;
		Array<JSONValue> array;
		JSONObject object;
	};
	s64 Number();
	String String();
	bool Boolean();
	Array<JSONValue> Array();
	JSONObject Object();
};

struct JSONObjectField
{
	String name;
	JSONValue value;
};

