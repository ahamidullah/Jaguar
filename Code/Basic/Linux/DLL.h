#pragma once

#include "../String.h"

struct DLL
{
	void *handle;
	String path;

	bool Close();
	void *Lookup(String sym, bool *err);
};

DLL OpenDLL(String path, bool *err);
