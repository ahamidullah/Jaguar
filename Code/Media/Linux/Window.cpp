#include "../Window.h"
#include "../Input.h"

#include "Code/Basic/Assert.h"
#include "Code/Basic/String.h"
#include "Code/Basic/Log.h"

struct WindowsContext
{
	Display *x11Display;
	s32 xinputOpcode;
} windowsContext;

s32 X11ErrorHandler(Display *display, XErrorEvent *event)
{
	char buffer[256];
	XGetErrorText(display, event->error_code, buffer, sizeof(buffer));
	Abort("X11 error: %s.", buffer);
	return 0;
}

void InitializeWindow(bool multithreaded)
{
	// Install a new error handler.
	// Note this error handler is global.  All display connections in all threads of a process use the same error handler.
	XSetErrorHandler(&X11ErrorHandler);

	if (multithreaded)
	{
		XInitThreads();
	}
}

Display *GetX11Display()
{
	return windowsContext.x11Display;
}

PlatformWindow CreateWindow(s64 width, s64 height, bool startFullscreen)
{
	windowsContext.x11Display = XOpenDisplay(NULL);
	if (!windowsContext.x11Display)
	{
		Abort("Failed to create display.");
	}
	auto screen = XDefaultScreen(windowsContext.x11Display);
	auto rootWindow = XRootWindow(windowsContext.x11Display, screen);

	// Initialize XInput2, which we require for raw input.
	{
		auto firstEventReturn = s32{};
		auto firstErrorReturn = s32{};
		if (!XQueryExtension(windowsContext.x11Display, "XInputExtension", &windowsContext.xinputOpcode, &firstEventReturn, &firstErrorReturn))
		{
			Abort("The X server does not support the XInput extension.");
		}

		// We are supposed to pass in the minimum version we require to XIQueryVersion, and it passes back what version is available.
		auto majorVersion = 2, minorVersion = 0;
		XIQueryVersion(windowsContext.x11Display, &majorVersion, &minorVersion);
		if (majorVersion < 2)
		{
			Abort("XInput version 2.0 or greater is required: version %d.%d is available.", majorVersion, minorVersion);
		}

		u8 mask[] = {0, 0, 0};
		auto eventMask = XIEventMask
		{
			.deviceid = XIAllMasterDevices,
			.mask_len = sizeof(mask),
			.mask = mask,
		};
		XISetMask(mask, XI_RawMotion);
		XISetMask(mask, XI_RawButtonPress);
		XISetMask(mask, XI_RawButtonRelease);
		XISetMask(mask, XI_RawKeyPress);
		XISetMask(mask, XI_RawKeyRelease);
		XISetMask(mask, XI_FocusOut);
		XISetMask(mask, XI_FocusIn);
		if (XISelectEvents(windowsContext.x11Display, rootWindow, &eventMask, 1) != Success)
		{
			Abort("Failed to select XInput events.");
		}
	}

	auto visualInfoTemplate = XVisualInfo
	{
		.screen = screen,
	};
	auto numberOfVisuals = 0;
	auto visualInfo = XGetVisualInfo(windowsContext.x11Display, VisualScreenMask, &visualInfoTemplate, &numberOfVisuals);
	Assert(visualInfo->c_class == TrueColor);

	auto windowAttributes = XSetWindowAttributes
	{
		.background_pixel = 0xFFFFFFFF,
		.border_pixmap = None,
		.border_pixel = 0,
		.event_mask = StructureNotifyMask,
		.colormap = XCreateColormap(windowsContext.x11Display, rootWindow, visualInfo->visual, AllocNone),
	};
	auto windowAttributesMask =
		CWBackPixel
		| CWColormap
		| CWBorderPixel
		| CWEventMask;
	auto result = PlatformWindow{};
	result.x11Window = XCreateWindow(
		windowsContext.x11Display,
		rootWindow,
		0,
		0,
		width,
		height,
		0,
		visualInfo->depth,
		InputOutput,
		visualInfo->visual,
		windowAttributesMask,
		&windowAttributes);
	if (!result.x11Window)
	{
		Abort("Failed to create a window.");
	}

	XFree(visualInfo);
	XStoreName(windowsContext.x11Display, result.x11Window, "Jaguar");
	XMapWindow(windowsContext.x11Display, result.x11Window);
	XFlush(windowsContext.x11Display);

	// Set up the "delete window atom" which signals to the application when the window is closed through the window manager UI.
	if ((result.x11DeleteWindowAtom = XInternAtom(windowsContext.x11Display, "WM_DELETE_WINDOW", 1)))
	{
		XSetWMProtocols(windowsContext.x11Display, result.x11Window, &result.x11DeleteWindowAtom, 1);
	}
	else
	{
		LogPrint(ERROR_LOG, "Unable to register WM_DELETE_WINDOW atom.\n");
	}

	// Get actual window dimensions without window borders.
	{
		s32 windowX, windowY;
		u32 windowWidth;
		u32 borderWidth, depth;
		if (!XGetGeometry(windowsContext.x11Display, result.x11Window, &rootWindow, &windowX, &windowY, &windowWidth, &result.height, &borderWidth, &depth))
		{
			Abort("Failed to get the screen's geometry.");
		}
	}

	// Create a blank cursor for when we want to hide the cursor.
	{
		XColor xcolor;
		static char cursorPixels[] = {};
		Pixmap pixmap = XCreateBitmapFromData(windowsContext.x11Display, result.x11Window, cursorPixels, 1, 1);
		result.x11BlankCursor = XCreatePixmapCursor(windowsContext.x11Display, pixmap, pixmap, &xcolor, &xcolor, 1, 1); 
		XFreePixmap(windowsContext.x11Display, pixmap);
	}

	return result;
}

WindowEvents ProcessWindowEvents(PlatformWindow *window, InputButtons *keyboard, Mouse *mouse)
{
	auto windowEvents = WindowEvents{};
	XEvent event;
	XGenericEventCookie *cookie = &event.xcookie;
	XIRawEvent *rawEvent;
	XFlush(windowsContext.x11Display);
	while (XPending(windowsContext.x11Display))
	{
		XNextEvent(windowsContext.x11Display, &event);
		if (event.type == ClientMessage && (Atom)event.xclient.data.l[0] == window->x11DeleteWindowAtom)
		{
			windowEvents.quit = true;
			break;
		}
		if (event.type == ConfigureNotify)
		{
			XConfigureEvent configureEvent = event.xconfigure;
			// @TODO Window resize.
			continue;
		}
		if (!XGetEventData(windowsContext.x11Display, cookie) || cookie->type != GenericEvent || cookie->extension != windowsContext.xinputOpcode)
		{
			continue;
		}
		rawEvent = (XIRawEvent *)cookie->data;
		switch(rawEvent->evtype)
		{
		case XI_RawMotion:
		{
			// @TODO: Check XIMaskIsSet(re->valuators.mask, 0) for x and XIMaskIsSet(re->valuators.mask, 1) for y.
			mouse->rawDeltaX += rawEvent->raw_values[0];
			mouse->rawDeltaY -= rawEvent->raw_values[1];
		} break;
		case XI_RawKeyPress:
		{
			PressButton(rawEvent->detail, keyboard);
		} break;
		case XI_RawKeyRelease:
		{
			ReleaseButton(rawEvent->detail, keyboard);
		} break;
		case XI_RawButtonPress:
		{
			auto buttonIndex = (event.xbutton.button - 1);
			if (buttonIndex > MOUSE_BUTTON_COUNT)
			{
				break;
			}
			PressButton(buttonIndex, &mouse->buttons);
		} break;
		case XI_RawButtonRelease:
		{
			auto buttonIndex = (event.xbutton.button - 1);
			if (buttonIndex > MOUSE_BUTTON_COUNT)
			{
				break;
			}
			ReleaseButton(buttonIndex, &mouse->buttons);
		} break;
		case XI_FocusIn:
		{
		} break;
		case XI_FocusOut:
		{
		} break;
		}
	}
	return windowEvents;
}

void ToggleFullscreen(PlatformWindow *window)
{
	XEvent event =
	{
		.xclient =
		{
			.type = ClientMessage,
			.window = window->x11Window,
			.message_type = XInternAtom(windowsContext.x11Display, "_NET_WM_STATE", True),
			.format = 32,
			.data =
			{
				.l =
				{
					2,
					(long int)XInternAtom(windowsContext.x11Display, "_NET_WM_STATE_FULLSCREEN", True),
					0,
					1,
					0,
				},
			},
		},
	};
	XSendEvent(windowsContext.x11Display, DefaultRootWindow(windowsContext.x11Display), False, SubstructureRedirectMask | SubstructureNotifyMask, &event);
}

void CaptureCursor(PlatformWindow *window)
{
	XDefineCursor(windowsContext.x11Display, window->x11Window, window->x11BlankCursor);
	XGrabPointer(windowsContext.x11Display, window->x11Window, True, 0, GrabModeAsync, GrabModeAsync, None, window->x11BlankCursor, CurrentTime);
}

void UncaptureCursor(PlatformWindow *window)
{
	XUndefineCursor(windowsContext.x11Display, window->x11Window);
	XUngrabPointer(windowsContext.x11Display, CurrentTime);
}

void DestroyWindow(PlatformWindow *window)
{
	XDestroyWindow(windowsContext.x11Display, window->x11Window);
	XCloseDisplay(windowsContext.x11Display);
}

#if defined(USE_VULKAN_RENDER_API)

const char *GetRequiredVulkanSurfaceInstanceExtension()
{
	return "VK_KHR_xlib_surface";
}

extern VkResult (*vkCreateXlibSurfaceKHR)(VkInstance, const VkXlibSurfaceCreateInfoKHR *, const VkAllocationCallbacks *, VkSurfaceKHR *);

VkResult CreateVulkanSurface(PlatformWindow *window, VkInstance instance, VkSurfaceKHR *surface)
{
	VkXlibSurfaceCreateInfoKHR surfaceCreateInfo =
	{
		.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
		.dpy = windowsContext.x11Display,
		.window = window->x11Window,
	};
	return vkCreateXlibSurfaceKHR(instance, &surfaceCreateInfo, NULL, surface);
}

#endif
