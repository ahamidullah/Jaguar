#pragma once

#if defined(__linux__)
	#include "Linux/Input.h"
#else
	#error Unsupported platform.
#endif

#include "Code/Common.h"

constexpr u32 SCANCODE_COUNT = 256;

struct InputButtons
{
	bool *down;
	bool *pressed;
	bool *released;
};

struct Mouse
{
	s32 wheel = 0;
	s32 x = 0;
	s32 y = 0;
	s32 deltaX = 0;
	s32 deltaY = 0;
	f32 rawDeltaX = 0;
	f32 rawDeltaY = 0;
	f32 sensitivity = 0.005;
	InputButtons buttons;
};

void InitializeInput();

WindowEvents GetInput(PlatformWindow *window);

void PressButton(s64 buttonIndex, InputButtons *buttons);
void ReleaseButton(s64 buttonIndex, InputButtons *buttons);

bool IsKeyDown(KeySymbol keySymbol);
bool WasKeyPressed(KeySymbol keySymbol);
bool WasKeyReleased(KeySymbol keySymbol);

f32 GetMouseX();
f32 GetMouseY();

f32 GetMouseDeltaX();
f32 GetMouseDeltaY();
f32 GetMouseSensitivity();
bool IsMouseButtonDown(MouseButton mouseButton);
bool WasMouseButtonPressed(MouseButton mouseButton);
bool WasMouseButtonReleased(MouseButton mouseButton);
