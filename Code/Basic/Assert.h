#pragma once

#include "Code/Common.h"

#define Assert(x) AssertActual(x, __FILE__, __func__, __LINE__, #x)
void AssertActual(bool test, const char *fileName, const char *functionName, s64 lineNumber, const char *testName);
