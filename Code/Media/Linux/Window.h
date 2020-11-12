#pragma once

#include "../PCH.h"
#include "Basic/String.h"
#include "Common.h"

struct Window
{
	xcb_screen_t *xcbScreen;
	xcb_window_t xcbWindow;
	xcb_intern_atom_reply_t *xcbDeleteWindowAtom;
	s64 height;
	bool isFullscreen;
	bool isCursorHidden;

	void SetName(string::String n);
	void SetIcon();
	void Fullscreen();
	void Unfullscreen();
	void ToggleFullscreen();
	bool IsFullscreen();
	void HideCursor();
	void UnhideCursor();
	void ToggleHideCursor();
	void Destroy();
};

Window NewWindow(s64 w, s64 h);
