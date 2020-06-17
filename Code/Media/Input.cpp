#include "Input.h"

#include "Code/Basic/Memory.h"

struct InputContext
{
	Mouse mouse;
	InputButtons keyboard;
	WindowEvents windowEvents;
} inputContext;

InputButtons CreateInputButtons(s64 size)
{
	auto buttons = InputButtons
	{
		.down = (bool *)AllocateMemory(size),
		.pressed = (bool *)AllocateMemory(size),
		.released = (bool *)AllocateMemory(size),
	};
	SetMemory(buttons.pressed, false, size);
	SetMemory(buttons.released, false, size);
	SetMemory(buttons.down, false, size);
	return buttons;
}

void InitializeInput()
{
	inputContext.mouse.buttons = CreateInputButtons(MOUSE_BUTTON_COUNT);
	inputContext.keyboard = CreateInputButtons(SCANCODE_COUNT);
}

// @TODO: Swap the order of parameters on all of these functions.

void PressButton(s64 buttonIndex, InputButtons *buttons)
{
	buttons->pressed[buttonIndex] = !buttons->down[buttonIndex];
	buttons->down[buttonIndex] = true;
}

void ReleaseButton(s64 buttonIndex, InputButtons *buttons)
{
	buttons->released[buttonIndex] = buttons->down[buttonIndex];
	buttons->down[buttonIndex] = false;
}

bool IsButtonDown(s64 buttonIndex, InputButtons *buttons)
{
	return buttons->down[buttonIndex];
}

bool WasButtonPressed(s64 buttonIndex, InputButtons *buttons)
{
	return buttons->pressed[buttonIndex];
}

bool WasButtonReleased(s64 buttonIndex, InputButtons *buttons)
{
	return buttons->released[buttonIndex];
}

bool IsKeyDown(KeySymbol keySymbol)
{
	s32 keyCode = KeySymbolToScancode(keySymbol);
	if (keyCode == 0)
	{
		return false;
	}
	return IsButtonDown(keyCode, &inputContext.keyboard);
}

bool WasKeyPressed(KeySymbol keySymbol)
{
	s32 keyCode = KeySymbolToScancode(keySymbol);
	if (keyCode == 0)
	{
		return false;
	}
	return WasButtonPressed(keyCode, &inputContext.keyboard);
}

bool WasKeyReleased(KeySymbol keySymbol)
{
	s32 keyCode = KeySymbolToScancode(keySymbol);
	if (keyCode == 0)
	{
		return false;
	}
	return WasButtonReleased(keyCode, &inputContext.keyboard);
}

bool IsMouseButtonDown(MouseButton mouseButton)
{
	if (mouseButton < 0 || mouseButton >= MOUSE_BUTTON_COUNT)
	{
		return false;
	}
	return IsButtonDown(mouseButton, &inputContext.mouse.buttons);
}

bool WasMouseButtonPressed(MouseButton mouseButton)
{
	if (mouseButton < 0 || mouseButton >= MOUSE_BUTTON_COUNT)
	{
		return false;
	}
	return WasButtonPressed(mouseButton, &inputContext.mouse.buttons);
}

bool WasMouseButtonReleased(MouseButton mouseButton)
{
	if (mouseButton < 0 || mouseButton >= MOUSE_BUTTON_COUNT)
	{
		return false;
	}
	return WasButtonReleased(mouseButton, &inputContext.mouse.buttons);
}

f32 GetMouseX()
{
	return inputContext.mouse.x;
}

f32 GetMouseY()
{
	return inputContext.mouse.y;
}

f32 GetMouseDeltaX()
{
	return inputContext.mouse.rawDeltaX;
}

f32 GetMouseDeltaY()
{
	return inputContext.mouse.rawDeltaY;
}

f32 GetMouseSensitivity()
{
	return inputContext.mouse.sensitivity;
}

bool QuitWindowEvent()
{
	return inputContext.windowEvents.quit;
}

WindowEvents GetInput(PlatformWindow *window)
{
	// Clear per-frame input.
	SetMemory(inputContext.mouse.buttons.pressed, false, sizeof(bool) * MOUSE_BUTTON_COUNT);
	SetMemory(inputContext.mouse.buttons.released, false, sizeof(bool) * MOUSE_BUTTON_COUNT);
	SetMemory(inputContext.keyboard.pressed, false, sizeof(bool) * SCANCODE_COUNT);
	SetMemory(inputContext.keyboard.released, false, sizeof(bool) * SCANCODE_COUNT);
	inputContext.mouse.rawDeltaX = 0;
	inputContext.mouse.rawDeltaY = 0;

	auto windowEvents = ProcessWindowEvents(window, &inputContext.keyboard, &inputContext.mouse);

	// Update mouse position.
	s32 oldX = inputContext.mouse.x;
	s32 oldY = inputContext.mouse.y;
	QueryMousePosition(window, &inputContext.mouse.x, &inputContext.mouse.y);
	inputContext.mouse.deltaX = inputContext.mouse.x - oldX;
	inputContext.mouse.deltaY = inputContext.mouse.y - oldY;

	return windowEvents;
}
