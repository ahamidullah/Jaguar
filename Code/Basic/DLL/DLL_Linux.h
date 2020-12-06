#pragma once

#include "Basic/String.h"

namespace dll
{

struct DLL
{
	void *handle;
	str::String path;

	bool Close();
	void *Lookup(str::String name, bool *err);
};

DLL Open(str::String path, bool *err);

}
