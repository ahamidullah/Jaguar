#include "../Window.h"
#include "../Platform.h"
#include "../Input.h"
#include "Basic/Assert.h"
#include "Basic/String.h"
#include "Basic/Log.h"

#if 0
Display *x11Display;
s32 xinputOpcode;

s32 X11ErrorHandler(Display *d, XErrorEvent *e)
{
	char buf[256];
	XGetErrorText(d, e->error_code, buf, sizeof(buf));
	Abort("X11 error: %s.", buf);
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
#endif

PlatformWindow NewWindow(s64 w, s64 h, bool fullscreen)
{
    auto conn = XCBConnection();
    auto setup = xcb_get_setup(conn);
    auto itr = xcb_setup_roots_iterator(setup);
    auto screen = itr.data;
	// Initialize XInput2, which we require for raw input.
	{
		auto ext = String{"XInputExtension"};
		auto cookie = xcb_query_extension(conn, ext.Length(), (const char *)&ext[0]);
		auto reply = xcb_query_extension_reply(conn, cookie, NULL);
		if (!reply->present)
		{
			// Xinput is not supported.
			Abort("Window", "The X server does not support the XInput extension.");
		}
		free(reply);
		struct
		{
			xcb_input_event_mask_t head;
			xcb_input_xi_event_mask_t mask;
		} mask;
		mask.head.deviceid = XCB_INPUT_DEVICE_ALL;
		mask.head.mask_len = sizeof(mask.mask) / sizeof(u32);
		mask.mask = (xcb_input_xi_event_mask_t)
			(XCB_INPUT_XI_EVENT_MASK_RAW_MOTION
			| XCB_INPUT_XI_EVENT_MASK_RAW_BUTTON_PRESS
			| XCB_INPUT_XI_EVENT_MASK_RAW_BUTTON_RELEASE
			| XCB_INPUT_XI_EVENT_MASK_RAW_KEY_PRESS
			| XCB_INPUT_XI_EVENT_MASK_RAW_KEY_RELEASE
			| XCB_INPUT_XI_EVENT_MASK_FOCUS_OUT
			| XCB_INPUT_XI_EVENT_MASK_FOCUS_IN);
		xcb_input_xi_select_events(conn, screen->root, 1, &mask.head);
	}
	// Create window.
    auto win = PlatformWindow{};
	{
    	auto valMask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    	auto valList = StaticArray<u32, 2>
    	{
    		screen->black_pixel,
    		XCB_EVENT_MASK_STRUCTURE_NOTIFY,
    	};
    	win.xcbHandle = xcb_generate_id(conn);
    	xcb_create_window(
    		conn,
			XCB_COPY_FROM_PARENT,
			win.xcbHandle,
			screen->root,
			0, 0, // x y
			w, h,
			10, // border width
			XCB_WINDOW_CLASS_INPUT_OUTPUT,
			screen->root_visual,
			valMask, &valList[0]);
    	auto cookie = xcb_intern_atom(conn, 1, 12, "WM_PROTOCOLS");
    	auto reply = xcb_intern_atom_reply(conn, cookie, 0);
    	auto cookie2 = xcb_intern_atom(conn, 0, 16, "WM_DELETE_WINDOW");
    	auto err = (xcb_generic_error_t *){};
    	win.xcbDeleteWindowAtom = xcb_intern_atom_reply(conn, cookie2, &err);
    	if (err)
    	{
    		Abort("Window", "Failed to register XCB window delete atom: error_code %hu, major_code: %hu, minor_code: %hu, sequence: %hu.", err->error_code, err->major_code, err->minor_code, err->sequence);
    	}
    	xcb_change_property(conn, XCB_PROP_MODE_REPLACE, win.xcbHandle, reply->atom, 4, 32, 1, &win.xcbDeleteWindowAtom->atom);
    	free(reply);
    	xcb_map_window(conn, win.xcbHandle);
    }
    xcb_flush(conn);
	return win;
#if 0
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
		LogError("Window", "Unable to register WM_DELETE_WINDOW atom.\n");
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
#endif
}

WindowEvents ProcessWindowEvents(PlatformWindow *w, InputButtons *kb, Mouse *m)
{
	auto winEvents = WindowEvents{};
	auto ev = (xcb_generic_event_t *){};
	while ((ev = xcb_poll_for_event(XCBConnection())))
	{
		if (ev->response_type == 0)
		{
			auto err = (xcb_generic_error_t *)ev;
			LogError("Window", "XCB error event: error_code: %hu, major_code: %hu, minor_code: %hu, sequence: %hu.\n", err->error_code, err->major_code, err->minor_code, err->sequence);
			free(ev);
			continue;
		}
		if ((ev->response_type & ~80) == XCB_GE_GENERIC) // XInput event.
		{
		    switch (((xcb_ge_event_t *)ev)->event_type)
		    {
         	case XCB_INPUT_RAW_MOTION:
      		{
      		} break;
         	case XCB_INPUT_RAW_KEY_PRESS:
      		{
      			auto k = (xcb_input_raw_key_press_event_t *)ev;
				kb->Press(k->detail);
      		} break;
         	case XCB_INPUT_RAW_KEY_RELEASE:
      		{
      			auto k = (xcb_input_raw_key_release_event_t *)ev;
				kb->Release(k->detail);
      		} break;
     		}
		}
		else
		{
			auto evCode = ev->response_type & 0x7f;
			switch (evCode)
			{
			case XCB_CONFIGURE_NOTIFY:
			{
				// @TODO
			} break;
			case XCB_CLIENT_MESSAGE:
			{
				if (((xcb_client_message_event_t *)ev)->data.data32[0] == w->xcbDeleteWindowAtom->atom)
				{
					winEvents.quit = true;
					break;
				}
			} break;
			}
		}
		free(ev);
	}
	return winEvents;
#if 0
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
#endif
}

void ToggleFullscreen(PlatformWindow *w)
{
#if 0
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
#endif
}

void CaptureCursor(PlatformWindow *w)
{
#if 0
	XDefineCursor(x11Display, w->x11Handle, w->x11BlankCursor);
	XGrabPointer(x11Display, w->x11Handle, True, 0, GrabModeAsync, GrabModeAsync, None, w->x11BlankCursor, CurrentTime);
#endif
}

void UncaptureCursor(PlatformWindow *w)
{
#if 0
	XUndefineCursor(x11Display, w->x11Handle);
	XUngrabPointer(x11Display, CurrentTime);
#endif
}

void DestroyWindow(PlatformWindow *w)
{
    xcb_destroy_window(XCBConnection(), w->xcbHandle);
    xcb_disconnect(XCBConnection());
    free(w->xcbDeleteWindowAtom);
}
