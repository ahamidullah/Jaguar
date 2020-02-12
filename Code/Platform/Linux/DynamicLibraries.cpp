#include <dlfcn.h>

#include "Platform/DynamicLibraries.h"

PlatformDynamicLibraryHandle PlatformOpenDynamicLibrary(const char *filename)
{
	void* library = dlopen(filename, RTLD_NOW | RTLD_LOCAL);
	if (!library)
	{
		Abort("Failed to load shared library: %s", dlerror());
	}
	return library;
}

void PlatformCloseDynamicLibrary(PlatformDynamicLibraryHandle library)
{
	s32 errorCode = dlclose(library);
	if (errorCode < 0)
	{
		LogPrint(ERROR_LOG, "Failed to close shared library: %s\n", dlerror());
	}
}

PlatformDynamicLibraryFunction PlatformGetDynamicLibraryFunction(PlatformDynamicLibraryHandle library, const char *functionName)
{
	void *function = dlsym(library, functionName);
	if (!function)
	{
		Abort("Failed to load shared library function %s", functionName);
	}
	return function;
}
