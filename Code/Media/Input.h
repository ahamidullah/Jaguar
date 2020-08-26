#pragma once

#if __linux__
	#include "Linux/Input.h"
#else
	#error Unsupported platform.
#endif
#include "Basic/Array.h"
#include "Common.h"

const auto ScancodeCount = u32{256};

void InitializeInput();
f32 MouseX();
f32 MouseY();
f32 MouseDeltaX();
f32 MouseDeltaY();
f32 MouseSensitivity();
bool IsMouseButtonDown(MouseButton b);
bool WasMouseButtonPressed(MouseButton b);
bool WasMouseButtonReleased(MouseButton b);
WindowEvents GetInput(PlatformWindow *w);
bool IsKeyDown(KeySymbol k);
bool WasKeyPressed(KeySymbol k);
bool WasKeyReleased(KeySymbol k);
