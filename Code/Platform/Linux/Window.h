#pragma once

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

struct PlatformWindow {
	Window x11;
	Atom deleteWindowAtom;
	Cursor blankCursor;
	s32 height;
};

enum PlatformKeySymbol {
	W_KEY = XK_w,
	A_KEY = XK_a,
	S_KEY = XK_s,
	D_KEY = XK_d,
	E_KEY = XK_e,
	G_KEY = XK_g,
	Q_KEY = XK_q,
	R_KEY = XK_r,
	F_KEY = XK_f,
	P_KEY = XK_p,
	L_KEY = XK_l,
	C_KEY = XK_c,
	J_KEY = XK_j,
	K_KEY = XK_k,
	I_KEY = XK_i,
	M_KEY = XK_m,
	BACKSPACE_KEY = XK_BackSpace,
	LCTRL_KEY = XK_Control_L,
	RCTRL_KEY = XK_Control_R,
	LALT_KEY = XK_Alt_L,
	RALT_KEY = XK_Alt_R,
	ESCAPE_KEY = XK_Escape,
};

enum PlatformMouseButton {
	MOUSE_BUTTON_LEFT = 0,
	MOUSE_BUTTON_MIDDLE,
	MOUSE_BUTTON_RIGHT,
	MOUSE_BUTTON_COUNT
};

void PlatformCreateWindow(s32 windowWidth, s32 windowHeight, bool startFullscreen);
void PlatformToggleFullscreen();
void PlatformCaptureCursor();
void PlatformUncaptureCursor();
void PlatformCleanupDisplay();
void PlatformProcessWindowEvents(PlatformWindow window, PlatformWindowEvents *windowEvents, PlatformInput *input);
