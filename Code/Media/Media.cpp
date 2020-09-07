#include "Platform.h"
#include "Input.h"
#include "Window.h"
#include "Basic/Basic.h"
#include "Basic/Log.h"
#include "Basic/Thread.h"

auto isMediaInitialized = false;

void InitializeMedia(bool multithreaded)
{
	if (ThreadCount() != 1)
	{
		// We require single-threading when running initialization.
		Abort("Media", "InitializeMedia was called by a multi-threaded process (call InitializeMedia before spawning any threads).");
	}
	if (!isBasicInitialized)
	{
		Abort("Media", "Basic library was not initialized.");
	}
	InitializePlatform();
	InitializeInput();
	isMediaInitialized = true;
}
