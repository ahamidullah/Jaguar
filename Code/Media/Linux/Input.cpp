#include "../Input.h"
#include "XCBConnection.h"
#include "Basic/Assert.h"
#include "Basic/Log.h"

auto keySymbols = (xcb_key_symbols_t *){};

void InitializePlatformInput()
{
    keySymbols = xcb_key_symbols_alloc(XCBConnection());
    if (!keySymbols)
    {
    	Abort("Input", "Failed xcb_key_symbols_alloc().");
    }
}

void RefreshKeyboardMapping(xcb_mapping_notify_event_t *ev)
{
	if (xcb_refresh_keyboard_mapping(keySymbols, ev) == 0)
	{
		LogError("Input", "Failed xcb_refresh_keyboard_mapping.");
	}
}

u32 KeySymbolToScancode(KeySymbol k)
{
	// @TODO: This is supposedly pretty slow, should we try to speed it up somehow?
	auto kc = xcb_key_symbols_get_keycode(keySymbols, k);
	if (!kc)
	{
		LogError("Input", "Failed xcb_key_symbols_get_keycode for KeySymbol %d.", k);
		return 0;
	}
	return *kc;
}

KeySymbol ScancodeToKeySymbol(u32 scancode)
{
	// @TODO
	return KeySymbol{};
}

void QueryMousePosition(Window *w, s32 *x, s32 *y)
{
    auto err = (xcb_generic_error_t *){};
	auto cookie = xcb_query_pointer(XCBConnection(), w->xcbWindow);
	auto reply = xcb_query_pointer_reply(XCBConnection(), cookie, &err);
	if (err)
	{
		LogError("Input", "Failed xcb_query_pointer_reply: error_code %hu, major_code: %hu, minor_code: %hu, sequence: %hu.", err->error_code, err->major_code, err->minor_code, err->sequence);
		return;
	}
	*x = reply->win_x;
	*y = reply->win_y;
	free(reply);
	*y = (w->height - *y); // Bottom left is zero for us, top left is zero for x11.
}
