#pragma once

struct String;

typedef void *DLLHandle;
typedef void *DLLFunction;

DLLHandle OpenDLL(const char *filename);
void CloseDLL(DLLHandle library);
DLLFunction GetDLLFunction(DLLHandle library, const char *functionName);
