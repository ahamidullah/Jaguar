#pragma once

#include "Basic/String.h"

struct DLL
{
	void *handle;
	string::String path;

	bool Close();
	void *LookupProcedure(string::String name, bool *err);
};

DLL OpenDLL(string::String path, bool *err);
