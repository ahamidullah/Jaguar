#pragma once

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
	InputButtons buttons =
	{
		AllocateArray(bool, MOUSE_BUTTON_COUNT),
		AllocateArray(bool, MOUSE_BUTTON_COUNT),
		AllocateArray(bool, MOUSE_BUTTON_COUNT),
	};
};

struct WindowEvents
{
	bool quit = false;
};

void GetPlatformInput(WindowContext *window);

void PressButton(u32 buttonIndex, InputButtons *buttons);
void ReleaseButton(u32 buttonIndex, InputButtons *buttons);

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

bool QuitWindowEvent();
