#include <dlfcn.h>

DLLHandle OpenDLL(const String &filename, bool *error)
{
	auto library = dlopen(&filename[0], RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND);
	if (!library)
	{
		LogPrint(ERROR_LOG, "Failed to open DLL %k: %s.\n", filename, dlerror());
		*error = true;
		return {};
	}
	*error = false;
	return library;
}

bool CloseDLL(DLLHandle library)
{
	if (dlclose(library) < 0)
	{
		LogPrint(ERROR_LOG, "Failed to close DLL: %s.\n", dlerror());
		return false;
	}
	return true;
}

DLLFunction GetDLLFunction(DLLHandle library, const String &functionName, bool *error)
{
	// According to https://linux.die.net/man/3/dlsym:
	// If the symbol is not found, in the specified library or any of the libraries that were
	// automatically loaded by dlopen() when that library was loaded, dlsym() returns NULL.  Since
	// the value of the symbol could actually be NULL (so that a NULL return from dlsym() need not
	// indicate an error), the correct way to test for an error is to call dlerror() to clear any
	// old error conditions, then call dlsym(), and then call dlerror() again, saving its return
	// value into a variable, and check whether this saved value is not NULL.
	dlerror();
	auto function = dlsym(library, &functionName[0]);
	auto errorString = dlerror();
	if (errorString)
	{
		LogPrint(ERROR_LOG, "Failed to load DLL function %k: %s.\n", functionName, errorString);
		*error = true;
		return {};
	}
	*error = false;
	return function;
}
