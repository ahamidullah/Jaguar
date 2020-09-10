#pragma once

#include "../PCH.h"
#include "../Window.h"

enum KeySymbol
{
	WKey = XK_w,
	AKey = XK_a,
	SKey = XK_s,
	DKey = XK_d,
	EKey = XK_e,
	GKey = XK_g,
	QKey = XK_q,
	RKey = XK_r,
	FKey = XK_f,
	PKey = XK_p,
	LKey = XK_l,
	CKey = XK_c,
	JKey = XK_j,
	KKey = XK_k,
	IKey = XK_i,
	MKey = XK_m,
	BackspaceKey = XK_BackSpace,
	LctrlKey = XK_Control_L,
	RctrlKey = XK_Control_R,
	LaltKey = XK_Alt_L,
	RaltKey = XK_Alt_R,
	EscapeKey = XK_Escape,
};

enum MouseButton
{
	MouseButtonLeft = 0,
	MouseButtonMiddle,
	MouseButtonRight,
	MouseButtonCount
};

void InitializePlatformInput();
u32 KeySymbolToScancode(KeySymbol k);
KeySymbol ScancodeToKeySymbol(u32 scancode);
void QueryMousePosition(PlatformWindow *w, s32 *x, s32 *y);
