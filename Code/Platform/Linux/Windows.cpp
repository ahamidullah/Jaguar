#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput2.h>

#include "Platform/Windows.h"
#include "Common/Log.h"

struct {
	Display *display;
	s32 xinputOpcode;
} linuxWindowsContext;

PlatformWindow PlatformCreateWindow(s32 width, s32 height, bool startFullscreen) {
	linuxWindowsContext.display = XOpenDisplay(NULL);
	if (!linuxWindowsContext.display) {
		Abort("Failed to create display");
	}
	auto screen = XDefaultScreen(linuxWindowsContext.display);
	auto rootWindow = XRootWindow(linuxWindowsContext.display, screen);

	// Initialize XInput2, which we require for raw input.
	{
		s32 firstEventReturn = 0;
		s32 firstErrorReturn = 0;
		if (!XQueryExtension(linuxWindowsContext.display, "XInputExtension", &linuxWindowsContext.xinputOpcode, &firstEventReturn, &firstErrorReturn)) {
			Abort("The X server does not support the XInput extension");
		}

		// We are supposed to pass in the minimum version we require to XIQueryVersion, and it passes back what version is available.
		s32 majorVersion = 2, minorVersion = 0;
		XIQueryVersion(linuxWindowsContext.display, &majorVersion, &minorVersion);
		if (majorVersion < 2) {
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
		if (XISelectEvents(linuxWindowsContext.display, rootWindow, &eventMask, 1) != Success) {
			Abort("failed to select XInput events");
		}
	}

	XVisualInfo visualInfoTemplate = {
		.screen = screen,
	};
	s32 numberOfVisuals;
	XVisualInfo *visualInfo = XGetVisualInfo(linuxWindowsContext.display, VisualScreenMask, &visualInfoTemplate, &numberOfVisuals);
	Assert(visualInfo->c_class == TrueColor);

	XSetWindowAttributes windowAttributes = {
		.background_pixel = 0xFFFFFFFF,
		.border_pixmap = None,
		.border_pixel = 0,
		.event_mask = StructureNotifyMask,
		.colormap = XCreateColormap(linuxWindowsContext.display, rootWindow, visualInfo->visual, AllocNone),
	};
	s32 windowAttributesMask = CWBackPixel
							   | CWColormap
							   | CWBorderPixel
							   | CWEventMask;
	s32 requested_window_width = 1200;
	s32 requested_window_height = 1000;
	PlatformWindow window;
	window.x11 = XCreateWindow(linuxWindowsContext.display,
	                           rootWindow,
	                           0,
	                           0,
	                           requested_window_width,
	                           requested_window_height,
	                           0,
	                           visualInfo->depth,
	                           InputOutput,
	                           visualInfo->visual,
	                           windowAttributesMask,
	                           &windowAttributes);
	if (!window.x11) {
		Abort("failed to create a window");
	}

	XFree(visualInfo);
	XStoreName(linuxWindowsContext.display, window.x11, "Jaguar");
	XMapWindow(linuxWindowsContext.display, window.x11);
	XFlush(linuxWindowsContext.display);

	// Set up the "delete window atom" which signals to the application when the window is closed through the window manager UI.
	if ((window.deleteWindowAtom = XInternAtom(linuxWindowsContext.display, "WM_DELETE_WINDOW", 1))) {
		XSetWMProtocols(linuxWindowsContext.display, window.x11, &window.deleteWindowAtom, 1);
	} else {
		LogPrint(ERROR_LOG, "Unable to register deleteWindowAtom atom.\n");
	}

	// Get actual window dimensions without window borders.
	{
		s32 windowX, windowY;
		u32 windowWidth;
		u32 borderWidth, depth;
		if (!XGetGeometry(linuxWindowsContext.display, window.x11, &rootWindow, &windowX, &windowY, &windowWidth, &window.height, &borderWidth, &depth)) {
			Abort("failed to get the screen's geometry");
		}
	}

	// Create a blank cursor for when we want to hide the cursor.
	{
		XColor xcolor;
		static char cursorPixels[] = {};
		Pixmap pixmap = XCreateBitmapFromData(linuxWindowsContext.display, window.x11, cursorPixels, 1, 1);
		window.blankCursor = XCreatePixmapCursor(linuxWindowsContext.display, pixmap, pixmap, &xcolor, &xcolor, 1, 1); 
		XFreePixmap(linuxWindowsContext.display, pixmap);
	}

	return window;
}

template <size_t Size> void PlatformPressButton(u32 buttonIndex, PlatformInputButtons<Size> *buttons);
template <size_t Size> void PlatformReleaseButton(u32 buttonIndex, PlatformInputButtons<Size> *buttons);

void PlatformProcessEvents(PlatformWindow window, PlatformWindowEvents *windowEvents, PlatformInput *input) {
	XEvent event;
	XGenericEventCookie *cookie = &event.xcookie;
	XIRawEvent *rawEvent;
	XFlush(linuxWindowsContext.display);
	while (XPending(linuxWindowsContext.display)) {
		XNextEvent(linuxWindowsContext.display, &event);
		if ((event.type == ClientMessage) && ((Atom)event.xclient.data.l[0] == window.deleteWindowAtom)) {
			windowEvents->quit = true;
			break;
		}
		if (event.type == ConfigureNotify) {
			XConfigureEvent configureEvent = event.xconfigure;
			// @TODO Window resize.
			continue;
		}
		if (!XGetEventData(linuxWindowsContext.display, cookie)
		 || cookie->type != GenericEvent
		 || cookie->extension != linuxWindowsContext.xinputOpcode) {
			continue;
		}
		rawEvent = (XIRawEvent *)cookie->data;
		switch(rawEvent->evtype) {
		case XI_RawMotion:
		{
			// @TODO: Check XIMaskIsSet(re->valuators.mask, 0) for x and XIMaskIsSet(re->valuators.mask, 1) for y.
			input->mouse.rawDeltaX += rawEvent->raw_values[0];
			input->mouse.rawDeltaY -= rawEvent->raw_values[1];
		} break;
		case XI_RawKeyPress:
		{
			PlatformPressButton(rawEvent->detail, &input->keyboard);
		} break;
		case XI_RawKeyRelease:
		{
			PlatformReleaseButton(rawEvent->detail, &input->keyboard);
		} break;
		case XI_RawButtonPress:
		{
			u32 buttonIndex = (event.xbutton.button - 1);
			if (buttonIndex > MOUSE_BUTTON_COUNT)
			{
				break;
			}
			PlatformPressButton(buttonIndex, &input->mouse.buttons);
		} break;
		case XI_RawButtonRelease:
		{
			u32 buttonIndex = (event.xbutton.button - 1);
			if (buttonIndex > MOUSE_BUTTON_COUNT)
			{
				break;
			}
			PlatformReleaseButton(buttonIndex, &input->mouse.buttons);
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

void PlatformToggleFullscreen(PlatformWindow window) {
	XEvent event = {
		.xclient = {
			.type = ClientMessage,
			.window = window.x11,
			.message_type = XInternAtom(linuxWindowsContext.display, "_NET_WM_STATE", True),
			.format = 32,
			.data = {
				.l = {
					2,
					(long int)XInternAtom(linuxWindowsContext.display, "_NET_WM_STATE_FULLSCREEN", True),
					0,
					1,
					0,
				},
			},
		},
	};
	XSendEvent(linuxWindowsContext.display, DefaultRootWindow(linuxWindowsContext.display), False, SubstructureRedirectMask | SubstructureNotifyMask, &event);
}

void PlatformCaptureCursor(PlatformWindow window) {
	XDefineCursor(linuxWindowsContext.display, window.x11, window.blankCursor);
	XGrabPointer(linuxWindowsContext.display, window.x11, True, 0, GrabModeAsync, GrabModeAsync, None, window.blankCursor, CurrentTime);
}

void PlatformUncaptureCursor(PlatformWindow window) {
	XUndefineCursor(linuxWindowsContext.display, window.x11);
	XUngrabPointer(linuxWindowsContext.display, CurrentTime);
}

void PlatformDestroyWindow(PlatformWindow window) {
	XDestroyWindow(linuxWindowsContext.display, window.x11);
	XCloseDisplay(linuxWindowsContext.display);
}

s32 PlatformX11ErrorHandler(Display *display, XErrorEvent *event) {
	char buffer[256];
	XGetErrorText(linuxWindowsContext.display, event->error_code, buffer, sizeof(buffer));
	Abort("X11 error: %s.", buffer);
	return 0;
}

s32 PlatformKeySymbolToScancode(PlatformKeySymbol keySymbol) {
	s32 scanCode = XKeysymToKeycode(linuxWindowsContext.display, keySymbol);
	Assert(scanCode > 0);
	return scanCode;
}

void PlatformGetMousePosition(PlatformWindow window, s32 *x, s32 *y) {
	s32 screenX, screenY;
	Window root, child;
	u32 mouseButtons;
	XQueryPointer(linuxWindowsContext.display, window.x11, &root, &child, &screenX, &screenY, x, y, &mouseButtons);
	*y = (window.height - *y); // Bottom left is zero for us, top left is zero for x11.
}

void PlatformInitializeWindows() {
	// Install a new error handler.
	// Note this error handler is global.  All display connections in all threads of a process use the same error handler.
	XSetErrorHandler(&PlatformX11ErrorHandler);
}

#if defined(USE_VULKAN_RENDER_API)

const char *PlatformGetRequiredVulkanSurfaceInstanceExtension() {
	return "VK_KHR_xlib_surface";
}

//VkResult vkCreateXlibSurfaceKHR(VkInstance, const VkXlibSurfaceCreateInfoKHR *, const VkAllocationCallbacks *, VkSurfaceKHR *);
extern VkResult (*vkCreateXlibSurfaceKHR)(VkInstance, const VkXlibSurfaceCreateInfoKHR *, const VkAllocationCallbacks *, VkSurfaceKHR *);

VkResult PlatformCreateVulkanSurface(PlatformWindow window, VkInstance instance, VkSurfaceKHR *surface) {
	VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
		.dpy = linuxWindowsContext.display,
		.window = window.x11,
	};
	return vkCreateXlibSurfaceKHR(instance, &surfaceCreateInfo, NULL, surface);
}

#endif
