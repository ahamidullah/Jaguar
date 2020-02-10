#pragma once

#include "Common/Common.h"

#if defined(__linux__)
	#include "Platform/Linux/Input.h"
#else
	#error unsupported platform
#endif

#define SCANCODE_COUNT 256

template <size_t Size>
struct PlatformInputButtons {
	bool down[Size];
	bool pressed[Size];
	bool released[Size];
};

struct PlatformMouse {
	s32 wheel;
	s32 x, y;
	s32 deltaX, deltaY;
	f32 rawDeltaX, rawDeltaY;
	f32 sensitivity;
	PlatformInputButtons<MOUSE_BUTTON_COUNT> buttons;
};

struct PlatformInput {
	Mouse mouse;
	PlatformInputButtons<SCANCODE_COUNT> keyboard;
	bool gotQuitEvent;
};

void PlatformGetInput(PlatformInput *input);

bool PlatformIsKeyDown(PlatformKeySymbol keySymbol, PlatformInput *input);
bool PlatformWasKeyPressed(PlatformKeySymbol keySymbol, PlatformInput *input);
bool PlatformWasKeyReleased(PlatformKeySymbol keySymbol, PlatformInput *input);

bool PlatformIsMouseButtonDown(PlatformMouseButton mouseButton, PlatformInput *input);
bool PlatformWasMouseButtonPressed(PlatformMouseButton mouseButton, PlatformInput *input);
bool PlatformWasMouseButtonReleased(PlatformMouseButton mouseButton, PlatformInput *input);
