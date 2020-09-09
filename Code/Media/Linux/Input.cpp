#include "../Input.h"
#include "XCBConnection.h"
#include "Basic/Assert.h"
#include "Basic/Log.h"

xcb_key_symbols_t *XCBKeySymbols()
{
    static auto syms = (xcb_key_symbols_t *){};
    if (!syms)
    {
    	syms = xcb_key_symbols_alloc(XCBConnection());
    	if (!syms)
    	{
    		Abort("Input", "Failed xcb_key_symbols_alloc().");
    	}
    }
    return syms;
}

s32 KeySymbolToScancode(KeySymbol k)
{
	auto kc = xcb_key_symbols_get_keycode(XCBKeySymbols(), k);
	if (!kc)
	{
		Abort("Input", "Failed xcb_key_symbols_get_keycode for KeySymbol %d.", k);
	}
	return *kc;
}

void ShutdownPlatformInput()
{
    xcb_key_symbols_free(XCBKeySymbols());
}

void QueryMousePosition(PlatformWindow *w, s32 *x, s32 *y)
{
#if 0
	auto screenX = s32{}, screenY = s32{};
	auto root = Window{}, child = Window{};
	auto mouseButtons = u32{};
	XQueryPointer(X11Display(), w->x11Handle, &root, &child, &screenX, &screenY, x, y, &mouseButtons);
	*y = (w->height - *y); // Bottom left is zero for us, top left is zero for x11.
#endif
}
