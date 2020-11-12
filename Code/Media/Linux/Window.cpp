#include "../Window.h"
#include "../Input.h"
#include "XCBConnection.h"
#include "Basic/Assert.h"
#include "Basic/String.h"
#include "Basic/Log.h"

Window NewWindow(s64 w, s64 h)
{
    auto conn = XCBConnection();
    auto setup = xcb_get_setup(conn);
    auto itr = xcb_setup_roots_iterator(setup);
    auto screen = itr.data;
	// Initialize XInput2, which we require for raw input.
	{
		auto ext = string::Make("XInputExtension");
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
		mask.mask = xcb_input_xi_event_mask_t(
			XCB_INPUT_XI_EVENT_MASK_RAW_MOTION
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
    	.xcbScreen = screen,
    	.height = h,
    };
	{
    	auto valMask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    	auto valList = array::MakeStatic<u32>(
    		screen->white_pixel,
    		XCB_EVENT_MASK_STRUCTURE_NOTIFY);
    	win.xcbWindow = xcb_generate_id(conn);
    	xcb_create_window(
    		conn,
			XCB_COPY_FROM_PARENT,
			win.xcbWindow,
			screen->root,
			0, 0, // x y
			w, h,
			10, // border width
			XCB_WINDOW_CLASS_INPUT_OUTPUT,
			screen->root_visual,
			valMask, &valList[0]);
    	auto err = (xcb_generic_error_t *){};
    	auto wmProtocolsCookie = xcb_intern_atom(conn, true, 12, "WM_PROTOCOLS");
    	auto wmProtocolsReply = xcb_intern_atom_reply(conn, wmProtocolsCookie, &err);
    	if (err)
    	{
    		Abort("Window", "Failed xcb_intern_atom WM_PROTOCOLS: error_code: %hu, major_code: %hu, minor_code: %hu, sequence: %hu.", err->error_code, err->major_code, err->minor_code, err->sequence);
    	}
    	auto wmDeleteCookie = xcb_intern_atom(conn, false, 16, "WM_DELETE_WINDOW");
    	win.xcbDeleteWindowAtom = xcb_intern_atom_reply(conn, wmDeleteCookie, &err);
    	if (err)
    	{
    		Abort("Window", "Failed xcb_intern_atom WM_DELETE_WINDOW: error_code: %hu, major_code: %hu, minor_code: %hu, sequence: %hu.", err->error_code, err->major_code, err->minor_code, err->sequence);
    	}
    	xcb_change_property(conn, XCB_PROP_MODE_REPLACE, win.xcbWindow, wmProtocolsReply->atom, 4, 32, 1, &win.xcbDeleteWindowAtom->atom);
    	free(wmProtocolsReply);
    	xcb_map_window(conn, win.xcbWindow);
    }
    xcb_flush(conn);
	return win;
}

void Window::SetName(string::String n)
{
}

void Window::SetIcon()
{
}

void Window::Fullscreen()
{
	if (!this->isFullscreen)
	{
		this->ToggleFullscreen();
	}
}
 
void Window::Unfullscreen()
{
	if (this->isFullscreen)
	{
		this->ToggleFullscreen();
	}
}

void Window::ToggleFullscreen()
{
	auto err = (xcb_generic_error_t *){};
    auto wmStateCookie = xcb_intern_atom(XCBConnection(), true, 13, "_NET_WM_STATE");
    auto wmStateReply = xcb_intern_atom_reply(XCBConnection(), wmStateCookie, &err); 
    if (err)
    {
    	LogError("Window", "Failed xcb_intern_atom _NET_WM_STATE: error_code: %hu, major_code: %hu, minor_code: %hu, sequence: %hu.", err->error_code, err->major_code, err->minor_code, err->sequence);
    	return;
    }
    auto wmFullscreenCookie = xcb_intern_atom(XCBConnection(), true, 24, "_NET_WM_STATE_FULLSCREEN");
    auto wmFullscreenReply = xcb_intern_atom_reply(XCBConnection(), wmFullscreenCookie, &err);
    if (err)
    {
    	LogError("Window", "Failed xcb_intern_atom _NET_WM_STATE_FULLSCREEN: error_code: %hu, major_code: %hu, minor_code: %hu, sequence: %hu.", err->error_code, err->major_code, err->minor_code, err->sequence);
    	return;
    }
	xcb_change_property(
		XCBConnection(),
		XCB_PROP_MODE_REPLACE,
		this->xcbWindow,
		wmStateReply->atom,
		XCB_ATOM_ATOM,
		32,
		1,
		&wmFullscreenReply->atom);
	auto ev = xcb_client_message_event_t
	{
	 	.response_type = XCB_CLIENT_MESSAGE,
		.format = 32,
		.window = this->xcbWindow,
		.type = wmStateReply->atom,
		.data.data32 =
		{
			2,
			wmFullscreenReply->atom,
			0,
			1,
			0,
		},
	};
	xcb_send_event(XCBConnection(), false, this->xcbWindow, XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_STRUCTURE_NOTIFY, (const char *)&ev);
	free(wmStateReply);
	free(wmFullscreenReply);
	this->isFullscreen = !this->isFullscreen;
}

bool Window::IsFullscreen()
{
	return this->isFullscreen;
}

void Window::HideCursor()
{
	auto bits = u8{0x00};
	auto pixmap = xcb_create_pixmap_from_bitmap_data(
		XCBConnection(),
		this->xcbWindow,
		&bits,
		1, 1,
		1,
		0,
		0,
		NULL);
	auto cursor = (xcb_cursor_t)xcb_generate_id(XCBConnection());
    xcb_create_cursor(
		XCBConnection(),
		cursor,
		pixmap,
		pixmap,
		65535, 65535, 65535,
		0, 0, 0,
		0, 0);
	xcb_free_pixmap(XCBConnection(), pixmap);
	auto grabCookie = xcb_grab_pointer(
		XCBConnection(),
		false,                  // get all pointer events specified by the following mask
		this->xcbScreen->root,  // grab the root window
		XCB_NONE,               // which events to let through
		XCB_GRAB_MODE_ASYNC,    // pointer events should continue as normal
		XCB_GRAB_MODE_ASYNC,    // keyboard mode
		XCB_NONE,               // confine_to = in which window should the cursor stay
		cursor,
		XCB_CURRENT_TIME);
	auto err = (xcb_generic_error_t *){};
	auto grabReply = xcb_grab_pointer_reply(XCBConnection(), grabCookie, &err);
	if (err)
	{
    	LogError("Window", "Failed xcb_grab_pointer: error_code: %hu, major_code: %hu, minor_code: %hu, sequence: %hu.", err->error_code, err->major_code, err->minor_code, err->sequence);
    	return;
	}
	this->isCursorHidden = true;
}

void Window::UnhideCursor()
{
	xcb_ungrab_pointer(XCBConnection(), XCB_CURRENT_TIME);
	this->isCursorHidden = false;
}

void Window::ToggleHideCursor()
{
	if (this->isCursorHidden)
	{
		this->UnhideCursor();
	}
	else
	{
		this->HideCursor();
	}
}

void Window::Destroy()
{
    xcb_destroy_window(XCBConnection(), this->xcbWindow);
    xcb_disconnect(XCBConnection());
    free(this->xcbDeleteWindowAtom);
}
