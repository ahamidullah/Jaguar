#include <dlfcn.h>

DLLHandle OpenDLL(const char *filename)
{
	void* library = dlopen(filename, RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND);
	if (!library)
	{
		Abort("failed to load shared library: %s", dlerror());
	}
	return library;
}

void CloseDLL(DLLHandle library)
{
	if (s32 errorCode = dlclose(library); errorCode < 0)
	{
		LogPrint(LogType::ERROR, "failed to close shared library: %s\n", dlerror());
	}
}

DLLFunction GetDLLFunction(DLLHandle library, const char *functionName)
{
	void *function = dlsym(library, &functionName[0]);
	if (!function)
	{
		Abort("failed to load shared library function %s", functionName);
	}
	return function;
}
