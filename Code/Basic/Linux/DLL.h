#pragma once

#include "../String.h"

struct DLL
{
	void *handle;
	String path;
};

typedef void *DLLFunction;

DLL OpenDLL(String path, bool *error);
bool CloseDLL(DLL dll);
DLLFunction GetDLLFunction(DLL dll, String functionName, bool *error);
