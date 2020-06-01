#include "Input.h"
#include "Window.h"

#include "Code/Basic/Basic.h"

void InitializeMedia(bool multithreaded)
{
	InitializeBasic();

	InitializeInput();
	InitializeWindow(multithreaded);
}
