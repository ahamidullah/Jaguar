#include "Input.h"
#include "Window.h"
#include "Basic/Basic.h"
#include "Basic/Log.h"

auto isMediaInitialized = false;

void InitializeMedia(bool multithreaded)
{
	if (!isBasicInitialized)
	{
		Abort("Basic library was not initialized.");
	}
	InitializeInput();
	InitializeWindows(multithreaded);
	isMediaInitialized = true;
}
