#pragma once

#include "Basic/String.h"

struct DLL
{
	void *handle;
	String path;

	bool Close();
	void *LookupProcedure(String name, bool *err);
};

DLL OpenDLL(String path, bool *err);
