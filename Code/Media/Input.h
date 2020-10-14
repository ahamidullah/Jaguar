#pragma once

#if __linux__
	#include "Linux/Input.h"
#else
	#error Unsupported platform.
#endif
#include "Event.h"
#include "Basic/Array.h"
#include "Common.h"

const auto ScancodeCount = u32{256};

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
