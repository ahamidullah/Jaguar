#pragma once

#include "Basic/PCH.h"
#ifdef __linux__
	#include <X11/keysym.h>
	#include <xcb/xcb.h>
	#include <xcb/xinput.h>
	#include <xcb/xcb_keysyms.h>
#else
	#error Unsupported platform.
#endif
