#pragma once

#include "../PCH.h"
#include "Common.h"

struct PlatformWindow
{
	xcb_window_t xcbHandle;
	xcb_intern_atom_reply_t *xcbDeleteWindowAtom;

#if 0
	Window x11Handle;
	Atom x11DeleteWindowAtom;
	Cursor x11BlankCursor;
	u32 height;
#endif
};

struct WindowEvents
{
	bool quit;
};

struct InputButtons;
struct Mouse;
void InitializeWindows(bool multithreaded);
PlatformWindow NewWindow(s64 w, s64 h, bool fullscreen);
WindowEvents ProcessWindowEvents(PlatformWindow *w, InputButtons *kb, Mouse *m);
void ToggleFullscreen(PlatformWindow *w);
void CaptureCursor(PlatformWindow *w);
void UncaptureCursor(PlatformWindow *w);
void DestroyWindow(PlatformWindow *w);
xcb_connection_t *XCBConnection();
