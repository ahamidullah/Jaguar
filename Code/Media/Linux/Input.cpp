#include "../Input.h"
#include "../Platform.h"
#include "Basic/Assert.h"
#include "Basic/Log.h"

xcb_connection_t *XCBConnection();

auto xcbKeySymbols = (xcb_key_symbols_t *){};

void InitializePlatformInput()
{
    xcbKeySymbols = xcb_key_symbols_alloc(XCBConnection());
    if (!xcbKeySymbols)
    {
    	Abort("Input", "Failed xcb_key_symbols_alloc().");
    }
}

s32 KeySymbolToScancode(KeySymbol k)
{
	auto kc = xcb_key_symbols_get_keycode(xcbKeySymbols, k);
	if (!kc)
	{
		Abort("Input", "Failed xcb_key_symbols_get_keycode for KeySymbol %d.", k);
	}
	return *kc;
}

void ShutdownPlatformInput()
{
    xcb_key_symbols_free(xcbKeySymbols);
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
