#pragma once

#if __linux__
	#include "Linux/Input.h"
#else
	#error Unsupported platform.
#endif
#include "Basic/Array.h"
#include "Common.h"

constexpr u32 ScancodeCount = 256;

// @TODO: Make these regular Arrays?
struct InputButtons
{
	Array<bool> down;
	Array<bool> pressed;
	Array<bool> released;
};

struct Mouse
{
	s32 wheel;
	s32 x;
	s32 y;
	s32 deltaX;
	s32 deltaY;
	f32 rawDeltaX;
	f32 rawDeltaY;
	f32 sensitivity;
	InputButtons buttons;
};

void InitializeInput();

WindowEvents GetInput(PlatformWindow *w);

void PressButton(InputButtons *b, s64 i);
void ReleaseButton(InputButtons *b, s64 i);

bool IsKeyDown(KeySymbol k);
bool WasKeyPressed(KeySymbol k);
bool WasKeyReleased(KeySymbol k);

f32 MouseX();
f32 MouseY();
f32 MouseDeltaX();
f32 MouseDeltaY();

f32 MouseSensitivity();

bool IsMouseButtonDown(MouseButton b);
bool WasMouseButtonPressed(MouseButton b);
bool WasMouseButtonReleased(MouseButton b);
