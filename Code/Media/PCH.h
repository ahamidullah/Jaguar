#pragma once

#include "Basic/PCH.h"
#ifdef __linux__
	#include <X11/keysym.h>
	#include <xcb/xcb.h>
	#include <xcb/xinput.h>
	#include <xcb/xcb_keysyms.h>
	#include <xcb/xcb_image.h>
#else
	#error Unsupported platform.
#endif
