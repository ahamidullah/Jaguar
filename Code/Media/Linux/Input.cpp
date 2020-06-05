#include "../Input.h"

#include "Code/Basic/Assert.h"
#include "Code/Basic/Log.h"

s32 KeySymbolToScancode(KeySymbol keySymbol)
{
	auto scancode = XKeysymToKeycode(GetX11Display(), keySymbol);
	Assert(scancode > 0);
	return scancode;
}

void QueryMousePosition(PlatformWindow *window, s32 *x, s32 *y)
{
	auto screenX = s32{}, screenY = s32{};
	auto root = Window{}, child = Window{};
	auto mouseButtons = u32{};
	XQueryPointer(GetX11Display(), window->x11Handle, &root, &child, &screenX, &screenY, x, y, &mouseButtons);
	*y = (window->height - *y); // Bottom left is zero for us, top left is zero for x11.
}
