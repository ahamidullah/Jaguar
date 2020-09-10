#pragma once

#include "../PCH.h"
#include "Common.h"

struct Window
{
	xcb_window_t xcbHandle;
	xcb_intern_atom_reply_t *xcbDeleteWindowAtom;
	s64 height;

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
Window NewWindow(s64 w, s64 h, bool fullscreen);
WindowEvents ProcessWindowEvents(Window *w, InputButtons *kb, Mouse *m);
void ToggleFullscreen(Window *w);
void CaptureCursor(Window *w);
void UncaptureCursor(Window *w);
void DestroyWindow(Window *w);
xcb_connection_t *XCBConnection();
