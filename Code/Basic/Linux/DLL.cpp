#include "../DLL.h"
#include "../Log.h"

DLL OpenDLL(String path, bool *error)
{
	auto result = DLL
	{
		.path = NewStringCopy(path),
	};
	result.handle = dlopen(&path[0], RTLD_NOW | RTLD_LOCAL);
	if (!result.handle)
	{
		LogPrint(ERROR_LOG, "Failed to open DLL %k: %s.\n", path, dlerror());
		*error = true;
		return {};
	}
	*error = false;
	return result;
}

bool CloseDLL(DLL dll)
{
	if (dlclose(dll.handle) < 0)
	{
		LogPrint(ERROR_LOG, "Failed to close DLL %k: %s.\n", dll.path, dlerror());
		return false;
	}
	return true;
}

DLLFunction GetDLLFunction(DLL dll, String functionName, bool *error)
{
	// According to https://linux.die.net/man/3/dlsym:
	//     "If the symbol is not found, in the specified library or any of the libraries that were
	//     automatically loaded by dlopen() when that library was loaded, dlsym() returns NULL.  Since
	//     the value of the symbol could actually be NULL (so that a NULL return from dlsym() need not
	//     indicate an error), the correct way to test for an error is to call dlerror() to clear any
	//     old error conditions, then call dlsym(), and then call dlerror() again, saving its return
	//     value into a variable, and check whether this saved value is not NULL."
	dlerror();
	auto function = dlsym(dll.handle, &functionName[0]);
	if (auto e = dlerror(); e)
	{
		LogPrint(ERROR_LOG, "Failed to load DLL function %k from file %k: %s.\n", functionName, dll.path, e);
		*error = true;
		return {};
	}
	*error = false;
	return function;
}
