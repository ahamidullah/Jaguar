#pragma once

#include "../String.h"

struct DLL
{
	void *handle;
	String path;
};

DLL OpenDLL(String path, bool *err);
bool CloseDLL(DLL dll);
void *LookupDLLSymbol(DLL dll, String sym, bool *err);
