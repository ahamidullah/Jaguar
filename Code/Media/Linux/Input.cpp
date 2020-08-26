#include "../Input.h"

#include "Basic/Assert.h"
#include "Basic/Log.h"

s32 KeySymbolToScancode(KeySymbol k)
{
	auto sc = XKeysymToKeycode(X11Display(), k);
	Assert(sc > 0);
	return sc;
}

void QueryMousePosition(PlatformWindow *w, s32 *x, s32 *y)
{
	auto screenX = s32{}, screenY = s32{};
	auto root = Window{}, child = Window{};
	auto mouseButtons = u32{};
	XQueryPointer(X11Display(), w->x11Handle, &root, &child, &screenX, &screenY, x, y, &mouseButtons);
	*y = (w->height - *y); // Bottom left is zero for us, top left is zero for x11.
}
