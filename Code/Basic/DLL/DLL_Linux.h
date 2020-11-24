#pragma once

#include "Basic/String.h"

namespace dll
{

struct DLL
{
	void *handle;
	string::String path;

	bool Close();
	void *Lookup(string::String name, bool *err);
};

DLL Open(string::String path, bool *err);

}
