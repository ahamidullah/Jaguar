#include "../Window.h"
#include "../Input.h"
#include "XCBConnection.h"
#include "Basic/Assert.h"
#include "Basic/String.h"
#include "Basic/Log.h"

Window NewWindow(s64 w, s64 h, bool fullscreen)
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
    auto win = Window
    {
    	.height = h,
    };
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
    	auto err = (xcb_generic_error_t *){};
    	auto cookie = xcb_intern_atom(conn, 1, 12, "WM_PROTOCOLS");
    	auto wmProtocolsReply = xcb_intern_atom_reply(conn, cookie, &err);
    	if (err)
    	{
    		Abort("Window", "Failed xcb_intern_atom WM_PROTOCOLS: error_code %hu, major_code: %hu, minor_code: %hu, sequence: %hu.", err->error_code, err->major_code, err->minor_code, err->sequence);
    	}
    	auto cookie2 = xcb_intern_atom(conn, 0, 16, "WM_DELETE_WINDOW");
    	win.xcbDeleteWindowAtom = xcb_intern_atom_reply(conn, cookie2, &err);
    	if (err)
    	{
    		Abort("Window", "Failed xcb_intern_atom WM_DELETE_WINDOW: error_code %hu, major_code: %hu, minor_code: %hu, sequence: %hu.", err->error_code, err->major_code, err->minor_code, err->sequence);
    	}
    	xcb_change_property(conn, XCB_PROP_MODE_REPLACE, win.xcbHandle, wmProtocolsReply->atom, 4, 32, 1, &win.xcbDeleteWindowAtom->atom);
    	free(wmProtocolsReply);
    	xcb_map_window(conn, win.xcbHandle);
    }
    xcb_flush(conn);
	return win;
}

WindowEvents ProcessWindowEvents(Window *w, InputButtons *kb, Mouse *m)
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
			case XCB_MAPPING_NOTIFY:
			{
				RefreshKeyboardMapping((xcb_mapping_notify_event_t *)ev);
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
}

void ToggleFullscreen(Window *w)
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

void CaptureCursor(Window *w)
{
#if 0
	XDefineCursor(x11Display, w->x11Handle, w->x11BlankCursor);
	XGrabPointer(x11Display, w->x11Handle, True, 0, GrabModeAsync, GrabModeAsync, None, w->x11BlankCursor, CurrentTime);
#endif
}

void UncaptureCursor(Window *w)
{
#if 0
	XUndefineCursor(x11Display, w->x11Handle);
	XUngrabPointer(x11Display, CurrentTime);
#endif
}

void DestroyWindow(Window *w)
{
    xcb_destroy_window(XCBConnection(), w->xcbHandle);
    xcb_disconnect(XCBConnection());
    free(w->xcbDeleteWindowAtom);
}
