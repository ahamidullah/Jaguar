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
	f32 sensitivity = 0;
	InputButtons buttons = {
		AllocateArray(bool, MOUSE_BUTTON_COUNT),
		AllocateArray(bool, MOUSE_BUTTON_COUNT),
		AllocateArray(bool, MOUSE_BUTTON_COUNT),
	};
};

struct Input
{
	Mouse mouse;
	InputButtons keyboard = {
		AllocateArray(bool, SCANCODE_COUNT),
		AllocateArray(bool, SCANCODE_COUNT),
		AllocateArray(bool, SCANCODE_COUNT),
	};
	struct
	{
		bool quit = false;
	} windowEvents;
};

void GetPlatformInput(WindowContext *window, Input *input);

void PressButton(u32 buttonIndex, InputButtons *buttons);
void ReleaseButton(u32 buttonIndex, InputButtons *buttons);

bool IsKeyDown(KeySymbol keySymbol, Input *input);
bool WasKeyPressed(KeySymbol keySymbol, Input *input);
bool WasKeyReleased(KeySymbol keySymbol, Input *input);

bool IsMouseButtonDown(MouseButton mouseButton, Input *input);
bool WasMouseButtonPressed(MouseButton mouseButton, Input *input);
bool WasMouseButtonReleased(MouseButton mouseButton, Input *input);
