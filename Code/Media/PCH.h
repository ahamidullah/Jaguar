#pragma once

#include "Basic/PCH.h"
#if __linux__
	#include <X11/X.h>
	#include <X11/Xlib.h>
	#include <X11/keysym.h>
	#include <X11/Xutil.h>
	#include <X11/Xatom.h>
	#include <X11/extensions/XInput2.h>
#else
	#error Unsupported platform.
#endif
