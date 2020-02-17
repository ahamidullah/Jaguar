#pragma once

#if defined(__linux__)
	#include "Platform/Linux/DynamicLibraries.h"
#else
	#error unsupported platform
#endif

PlatformDynamicLibraryHandle PlatformOpenDynamicLibrary(const char *filename);
void PlatformCloseDynamicLibrary(PlatformDynamicLibraryHandle library);
PlatformDynamicLibraryFunction PlatformGetDynamicLibraryFunction(PlatformDynamicLibraryHandle library, const char *functionName);
