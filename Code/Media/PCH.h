#pragma once

#include "Code/Basic/PCH.h"

#if defined(__linux__)
	// Window, Input
	#include <X11/X.h>
	#include <X11/Xlib.h>
	#include <X11/keysym.h>
	#include <X11/Xutil.h>
	#include <X11/Xatom.h>
	#include <X11/extensions/XInput2.h>

	// Window
	#define VK_NO_PROTOTYPES
	#include <vulkan/vulkan.h>
	#include <vulkan/vulkan_xlib.h> 
#else
	#error Unsupported platform.
#endif
