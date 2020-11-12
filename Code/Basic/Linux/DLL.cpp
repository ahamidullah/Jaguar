#include "../DLL.h"
#include "../Log.h"

DLL OpenDLL(string::String path, bool *err)
{
	auto dll = DLL
	{
		.path = path,
	};
	dll.handle = dlopen(path.ToCString(), RTLD_NOW | RTLD_LOCAL);
	if (!dll.handle)
	{
		LogError("DLL", "Failed to open DLL %k: %s.\n", path, dlerror());
		*err = true;
		return DLL{};
	}
	return dll;
}

bool DLL::Close()
{
	if (dlclose(this->handle) < 0)
	{
		LogError("DLL", "Failed to close DLL %k: %s.\n", this->path, dlerror());
		return false;
	}
	return true;
}

void *DLL::LookupProcedure(string::String name, bool *err)
{
	// According to https://linux.die.net/man/3/dlsym:
	//     "If the symbol is not found, in the specified library or any of the libraries that were
	//     automatically loaded by dlopen() when that library was loaded, dlsym() returns NULL.  Since
	//     the value of the symbol could actually be NULL (so that a NULL return from dlsym() need not
	//     indicate an error), the correct way to test for an error is to call dlerror() to clear any
	//     old error conditions, then call dlsym(), and then call dlerror() again, saving its return
	//     value into a variable, and check whether this saved value is not NULL."
	dlerror();
	auto ptr = dlsym(this->handle, name.ToCString());
	if (auto errStr = dlerror(); errStr)
	{
		LogError("DLL", "Failed to find DLL symbol %k in file %k: %s.\n", name, this->path, errStr);
		*err = true;
		return (void *){};
	}
	return ptr;
}
