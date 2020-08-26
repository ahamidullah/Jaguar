#include "Input.h"
#include "Window.h"
#include "Basic/Basic.h"

void InitializeMedia(bool multithreaded)
{
	InitializeBasic();
	InitializeInput();
	InitializeWindows(multithreaded);
}
