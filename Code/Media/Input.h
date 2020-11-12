#pragma once

#if __linux__
	#include "Linux/Input.h"
#else
	#error Unsupported platform.
#endif
#include "Event.h"
#include "Basic/Container/Array.h"
#include "Common.h"

const auto ScancodeCount = u32{256};

struct InputButtons
{
	array::Array<bool> down;
	array::Array<bool> pressed;
	array::Array<bool> released;

	void Press(s64 i);
	void Release(s64 i);
	bool Down(s64 i);
	bool Pressed(s64 i);
	bool Released(s64 i);
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
s32 MouseX();
s32 MouseY();
f32 MouseDeltaX();
f32 MouseDeltaY();
f32 MouseSensitivity();
bool MouseButtonDown(MouseButton b);
bool MouseButtonPressed(MouseButton b);
bool MouseButtonReleased(MouseButton b);
struct PlatformEvents;
PlatformEvents ProcessInput(Window *w);
bool KeyDown(KeySymbol k);
bool KeyPressed(KeySymbol k);
bool KeyReleased(KeySymbol k);
