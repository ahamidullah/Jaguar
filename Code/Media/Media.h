#pragma once

#include "Basic/Basic.h"

#if defined(__linux__)
	#include "Linux/Window.h"
	#include "Linux/Input.h"
#else
	#error unsupported platform
#endif

#include "Input.h"

void InitializeMedia(bool multithreaded = false, s64 maxFiberCount = 0);
