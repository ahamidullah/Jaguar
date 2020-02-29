#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput2.h>

static Display *x11Display;
static s32 xinputOpcode;

WindowContext CreateWindow(s32 width, s32 height, bool startFullscreen)
{
	x11Display = XOpenDisplay(NULL);
	if (!x11Display)
	{
		Abort("Failed to create display");
	}
	auto screen = XDefaultScreen(x11Display);
	auto rootWindow = XRootWindow(x11Display, screen);

	// Initialize XInput2, which we require for raw input.
	{
		s32 firstEventReturn = 0;
		s32 firstErrorReturn = 0;
		if (!XQueryExtension(x11Display, "XInputExtension", &xinputOpcode, &firstEventReturn, &firstErrorReturn))
		{
			Abort("The X server does not support the XInput extension");
		}

		// We are supposed to pass in the minimum version we require to XIQueryVersion, and it passes back what version is available.
		s32 majorVersion = 2, minorVersion = 0;
		XIQueryVersion(x11Display, &majorVersion, &minorVersion);
		if (majorVersion < 2)
		{
			Abort("XInput version 2.0 or greater is required: version %d.%d is available", majorVersion, minorVersion);
		}

		u8 mask[] = {0, 0, 0};
		XIEventMask eventMask = {
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
		if (XISelectEvents(x11Display, rootWindow, &eventMask, 1) != Success)
		{
			Abort("failed to select XInput events");
		}
	}

	XVisualInfo visualInfoTemplate = {
		.screen = screen,
	};
	s32 numberOfVisuals;
	XVisualInfo *visualInfo = XGetVisualInfo(x11Display, VisualScreenMask, &visualInfoTemplate, &numberOfVisuals);
	Assert(visualInfo->c_class == TrueColor);

	XSetWindowAttributes windowAttributes = {
		.background_pixel = 0xFFFFFFFF,
		.border_pixmap = None,
		.border_pixel = 0,
		.event_mask = StructureNotifyMask,
		.colormap = XCreateColormap(x11Display, rootWindow, visualInfo->visual, AllocNone),
	};
	s32 windowAttributesMask = CWBackPixel
							   | CWColormap
							   | CWBorderPixel
							   | CWEventMask;
	WindowContext window;
	window.x11 = XCreateWindow(x11Display,
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
	if (!window.x11)
	{
		Abort("failed to create a window");
	}

	XFree(visualInfo);
	XStoreName(x11Display, window.x11, "Jaguar");
	XMapWindow(x11Display, window.x11);
	XFlush(x11Display);

	// Set up the "delete window atom" which signals to the application when the window is closed through the window manager UI.
	if ((window.deleteWindowAtom = XInternAtom(x11Display, "WM_DELETE_WINDOW", 1)))
	{
		XSetWMProtocols(x11Display, window.x11, &window.deleteWindowAtom, 1);
	}
	else
	{
		LogPrint(LogType::ERROR, "unable to register deleteWindowAtom atom.\n");
	}

	// Get actual window dimensions without window borders.
	{
		s32 windowX, windowY;
		u32 windowWidth;
		u32 borderWidth, depth;
		if (!XGetGeometry(x11Display, window.x11, &rootWindow, &windowX, &windowY, &windowWidth, &window.height, &borderWidth, &depth))
		{
			Abort("failed to get the screen's geometry");
		}
	}

	// Create a blank cursor for when we want to hide the cursor.
	{
		XColor xcolor;
		static char cursorPixels[] = {};
		Pixmap pixmap = XCreateBitmapFromData(x11Display, window.x11, cursorPixels, 1, 1);
		window.blankCursor = XCreatePixmapCursor(x11Display, pixmap, pixmap, &xcolor, &xcolor, 1, 1); 
		XFreePixmap(x11Display, pixmap);
	}

	return window;
}

void ProcessWindowEvents(WindowContext *window, InputButtons *keyboard, Mouse *mouse, WindowEvents *windowEvents)
{
	XEvent event;
	XGenericEventCookie *cookie = &event.xcookie;
	XIRawEvent *rawEvent;
	XFlush(x11Display);
	while (XPending(x11Display)) {
		XNextEvent(x11Display, &event);
		if ((event.type == ClientMessage) && ((Atom)event.xclient.data.l[0] == window->deleteWindowAtom))
		{
			windowEvents->quit = true;
			break;
		}
		if (event.type == ConfigureNotify)
		{
			XConfigureEvent configureEvent = event.xconfigure;
			// @TODO Window resize.
			continue;
		}
		if (!XGetEventData(x11Display, cookie)
		 || cookie->type != GenericEvent
		 || cookie->extension != xinputOpcode)
		{
			continue;
		}
		rawEvent = (XIRawEvent *)cookie->data;
		switch(rawEvent->evtype) {
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
			u32 buttonIndex = (event.xbutton.button - 1);
			if (buttonIndex > MOUSE_BUTTON_COUNT)
			{
				break;
			}
			PressButton(buttonIndex, &mouse->buttons);
		} break;
		case XI_RawButtonRelease:
		{
			u32 buttonIndex = (event.xbutton.button - 1);
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
}

void ToggleFullscreen(WindowContext *window)
{
	XEvent event = {
		.xclient = {
			.type = ClientMessage,
			.window = window->x11,
			.message_type = XInternAtom(x11Display, "_NET_WM_STATE", True),
			.format = 32,
			.data = {
				.l = {
					2,
					(long int)XInternAtom(x11Display, "_NET_WM_STATE_FULLSCREEN", True),
					0,
					1,
					0,
				},
			},
		},
	};
	XSendEvent(x11Display, DefaultRootWindow(x11Display), False, SubstructureRedirectMask | SubstructureNotifyMask, &event);
}

void CaptureCursor(WindowContext *window)
{
	XDefineCursor(x11Display, window->x11, window->blankCursor);
	XGrabPointer(x11Display, window->x11, True, 0, GrabModeAsync, GrabModeAsync, None, window->blankCursor, CurrentTime);
}

void UncaptureCursor(WindowContext *window)
{
	XUndefineCursor(x11Display, window->x11);
	XUngrabPointer(x11Display, CurrentTime);
}

void DestroyWindow(WindowContext *window)
{
	XDestroyWindow(x11Display, window->x11);
	XCloseDisplay(x11Display);
}

s32 X11ErrorHandler(Display *display, XErrorEvent *event)
{
	char buffer[256];
	XGetErrorText(x11Display, event->error_code, buffer, sizeof(buffer));
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

#if defined(USE_VULKAN_RENDER_API)

const char *GetRequiredVulkanSurfaceInstanceExtension()
{
	return "VK_KHR_xlib_surface";
}

extern VkResult (*vkCreateXlibSurfaceKHR)(VkInstance, const VkXlibSurfaceCreateInfoKHR *, const VkAllocationCallbacks *, VkSurfaceKHR *);

VkResult CreateVulkanSurface(WindowContext *window, VkInstance instance, VkSurfaceKHR *surface)
{
	VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
		.dpy = x11Display,
		.window = window->x11,
	};
	return vkCreateXlibSurfaceKHR(instance, &surfaceCreateInfo, NULL, surface);
}

#endif
