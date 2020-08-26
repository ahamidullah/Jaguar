#include "../Window.h"
#include "../Input.h"
#include "Basic/Assert.h"
#include "Basic/String.h"
#include "Basic/Log.h"

Display *x11Display;
s32 xinputOpcode;

s32 X11ErrorHandler(Display *d, XErrorEvent *e)
{
	char buf[256];
	XGetErrorText(d, e->error_code, buf, sizeof(buf));
	Abort("Window", "X11 error: %s.", buf);
	return 0;
}

void InitializeWindows(bool multithreaded)
{
	// Install a new error handler.
	// Note this error handler is global.  All display connections in all threads of a process use the same error handler.
	XSetErrorHandler(&X11ErrorHandler);
	if (multithreaded)
	{
		XInitThreads();
	}
}

Display *X11Display()
{
	return x11Display;
}

PlatformWindow NewWindow(s64 w, s64 h, bool fullscreen)
{
	x11Display = XOpenDisplay(NULL);
	if (!x11Display)
	{
		Abort("Window", "Failed to create display.");
	}
	auto screen = XDefaultScreen(x11Display);
	auto rootWin = XRootWindow(x11Display, screen);
	// Initialize XInput2, which we require for raw input.
	{
		auto junk1 = s32{};
		auto junk2 = s32{};
		if (!XQueryExtension(x11Display, "XInputExtension", &xinputOpcode, &junk1, &junk2))
		{
			Abort("Window", "The X server does not support the XInput extension.");
		}

		// We are supposed to pass in the minimum version we require to XIQueryVersion, and it passes back what version is available.
		auto majorVer = 2, minorVer = 0;
		XIQueryVersion(x11Display, &majorVer, &minorVer);
		if (majorVer < 2)
		{
			Abort("Window", "XInput version 2.0 or greater is required: version %d.%d is available.", majorVer, minorVer);
		}

		auto mask = MakeStaticArray<u8>(0, 0, 0);
		auto eventMask = XIEventMask
		{
			.deviceid = XIAllMasterDevices,
			.mask_len = sizeof(mask.elements),
			.mask = mask.elements,
		};
		XISetMask(mask.elements, XI_RawMotion);
		XISetMask(mask.elements, XI_RawButtonPress);
		XISetMask(mask.elements, XI_RawButtonRelease);
		XISetMask(mask.elements, XI_RawKeyPress);
		XISetMask(mask.elements, XI_RawKeyRelease);
		XISetMask(mask.elements, XI_FocusOut);
		XISetMask(mask.elements, XI_FocusIn);
		if (XISelectEvents(x11Display, rootWin, &eventMask, 1) != Success)
		{
			Abort("Window", "Failed to select XInput events.");
		}
	}
	auto vis = XVisualInfo
	{
		.screen = screen,
	};
	auto numVis = 0;
	auto visInfo = XGetVisualInfo(x11Display, VisualScreenMask, &vis, &numVis);
	Assert(visInfo->c_class == TrueColor);
	auto winAttrs = XSetWindowAttributes
	{
		.background_pixel = 0xFFFFFFFF,
		.border_pixmap = None,
		.border_pixel = 0,
		.event_mask = StructureNotifyMask,
		.colormap = XCreateColormap(x11Display, rootWin, visInfo->visual, AllocNone),
	};
	auto winAttrMask =
		CWBackPixel
		| CWColormap
		| CWBorderPixel
		| CWEventMask;
	auto win = PlatformWindow{};
	win.x11Handle = XCreateWindow(
		x11Display,
		rootWin,
		0,
		0,
		w,
		h,
		0,
		visInfo->depth,
		InputOutput,
		visInfo->visual,
		winAttrMask,
		&winAttrs);
	if (!win.x11Handle)
	{
		Abort("Window", "Failed to create a window.");
	}
	XFree(visInfo);
	XStoreName(x11Display, win.x11Handle, "Jaguar");
	XMapWindow(x11Display, win.x11Handle);
	XFlush(x11Display);
	// Set up the "delete window atom" which signals to the application when the window is closed through the window manager UI.
	if ((win.x11DeleteWindowAtom = XInternAtom(x11Display, "WM_DELETE_WINDOW", 1)))
	{
		XSetWMProtocols(x11Display, win.x11Handle, &win.x11DeleteWindowAtom, 1);
	}
	else
	{
		LogPrint(ErrorLog, "Window", "Unable to register WM_DELETE_WINDOW atom.\n");
	}
	// Get actual window dimensions without window borders.
	{
		auto winX = s32{}, winY = s32{};
		auto winW = u32{};
		auto borderW = u32{};
		auto depth = u32{};
		if (!XGetGeometry(x11Display, win.x11Handle, &rootWin, &winX, &winY, &winW, &win.height, &borderW, &depth))
		{
			Abort("Window", "Failed to get the screen's geometry.");
		}
	}
	// Create a blank cursor for when we want to hide the cursor.
	{
		auto col = XColor{};
		auto cursorPix = MakeStaticArray<char>(0);
		auto pixmap = XCreateBitmapFromData(x11Display, win.x11Handle, cursorPix.elements, 1, 1);
		win.x11BlankCursor = XCreatePixmapCursor(x11Display, pixmap, pixmap, &col, &col, 1, 1); 
		XFreePixmap(x11Display, pixmap);
	}
	return win;
}

WindowEvents ProcessWindowEvents(PlatformWindow *w, InputButtons *kb, Mouse *m)
{
	auto winEvents = WindowEvents{};
	auto xEvent = XEvent{};
	auto cookie = &xEvent.xcookie;
	auto rawXEvent = (XIRawEvent *){};
	XFlush(x11Display);
	while (XPending(x11Display))
	{
		XNextEvent(x11Display, &xEvent);
		if (xEvent.type == ClientMessage && (Atom)xEvent.xclient.data.l[0] == w->x11DeleteWindowAtom)
		{
			winEvents.quit = true;
			break;
		}
		if (xEvent.type == ConfigureNotify)
		{
			//auto config = event.xconfigure;
			// @TODO Window resize.
			continue;
		}
		if (!XGetEventData(x11Display, cookie) || cookie->type != GenericEvent || cookie->extension != xinputOpcode)
		{
			continue;
		}
		rawXEvent = (XIRawEvent *)cookie->data;
		switch(rawXEvent->evtype)
		{
		case XI_RawMotion:
		{
			// @TODO: Check XIMaskIsSet(re->valuators.mask, 0) for x and XIMaskIsSet(re->valuators.mask, 1) for y.
			m->rawDeltaX += rawXEvent->raw_values[0];
			m->rawDeltaY -= rawXEvent->raw_values[1];
		} break;
		case XI_RawKeyPress:
		{
			kb->Press(rawXEvent->detail);
		} break;
		case XI_RawKeyRelease:
		{
			kb->Release(rawXEvent->detail);
		} break;
		case XI_RawButtonPress:
		{
			auto i = (xEvent.xbutton.button - 1);
			if (i > MouseButtonCount)
			{
				break;
			}
			m->buttons.Press(i);
		} break;
		case XI_RawButtonRelease:
		{
			auto i = (xEvent.xbutton.button - 1);
			if (i > MouseButtonCount)
			{
				break;
			}
			m->buttons.Release(i);
		} break;
		case XI_FocusIn:
		{
		} break;
		case XI_FocusOut:
		{
		} break;
		}
	}
	return winEvents;
}

void ToggleFullscreen(PlatformWindow *w)
{
	auto ev = XEvent
	{
		.xclient =
		{
			.type = ClientMessage,
			.window = w->x11Handle,
			.message_type = XInternAtom(x11Display, "_NET_WM_STATE", True),
			.format = 32,
			.data =
			{
				.l =
				{
					2,
					(long int)XInternAtom(x11Display, "_NET_WM_STATE_FULLSCREEN", True),
					0,
					1,
					0,
				},
			},
		},
	};
	XSendEvent(x11Display, DefaultRootWindow(x11Display), False, SubstructureRedirectMask | SubstructureNotifyMask, &ev);
}

void CaptureCursor(PlatformWindow *w)
{
	XDefineCursor(x11Display, w->x11Handle, w->x11BlankCursor);
	XGrabPointer(x11Display, w->x11Handle, True, 0, GrabModeAsync, GrabModeAsync, None, w->x11BlankCursor, CurrentTime);
}

void UncaptureCursor(PlatformWindow *w)
{
	XUndefineCursor(x11Display, w->x11Handle);
	XUngrabPointer(x11Display, CurrentTime);
}

void DestroyWindow(PlatformWindow *w)
{
	XDestroyWindow(x11Display, w->x11Handle);
	XCloseDisplay(x11Display);
}
