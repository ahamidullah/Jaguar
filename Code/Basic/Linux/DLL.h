#pragma once

#include "../String.h"

struct DLL
{
	void *handle;
	String path;

	bool Close();
	void *Symbol(String name, bool *err);
};

DLL OpenDLL(String path, bool *err);
