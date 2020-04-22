#include "Media.h"

#if defined(__linux__)
	#include "Linux/Window.cpp"
	#include "Linux/Input.cpp"
#else
	#error unsupported platform
#endif

#include "Input.cpp"

void InitializeMedia(bool multithreaded, s64 maxFiberCount)
{
	InitializeBasic(maxFiberCount);
	InitializeInput();
	InitializeWindow(multithreaded);
}
