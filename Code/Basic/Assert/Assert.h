#pragma once

#include "Common.h"

#define Assert(x) AssertActual(x, __FILE__, __func__, __LINE__, #x)
void AssertActual(bool test, const char *file, const char *func, s64 line, const char *src);
