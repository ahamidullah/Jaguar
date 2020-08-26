#pragma once

#include "../PCH.h"

#include "Common.h"

struct PlatformWindow
{
	Window x11Handle;
	Atom x11DeleteWindowAtom;
	Cursor x11BlankCursor;
	u32 height;
};

struct WindowEvents
{
	bool quit;
};

struct InputButtons;
struct Mouse;

void InitializeWindows(bool multithreaded);

PlatformWindow NewWindow(s64 width, s64 height, bool fullscreen);
WindowEvents ProcessWindowEvents(PlatformWindow *w, InputButtons *keys, Mouse *m);
void ToggleFullscreen(PlatformWindow *w);
void CaptureCursor(PlatformWindow *w);
void UncaptureCursor(PlatformWindow *w);
void DestroyWindow(PlatformWindow *w);

Display *X11Display();
