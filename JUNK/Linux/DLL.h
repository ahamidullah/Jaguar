#pragma once

struct String;

typedef void *DLLHandle;
typedef void *DLLFunction;

DLLHandle OpenDLL(const String &filename, bool *error);
bool CloseDLL(DLLHandle library);
DLLFunction GetDLLFunction(DLLHandle library, const String &functionName, bool *error);
