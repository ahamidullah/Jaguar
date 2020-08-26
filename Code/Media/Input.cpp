#include "Input.h"

#include "Basic/Memory.h"

struct Input
{
	Mouse mouse;
	InputButtons keyboard;
	WindowEvents windowEvents;
} input;

InputButtons NewInputButtons(s64 size)
{
	auto buttons = InputButtons
	{
		.down = NewArrayWithCount<bool>(size),
		.pressed = NewArrayWithCount<bool>(size),
		.released = NewArrayWithCount<bool>(size),
	};
	SetMemory(&buttons.pressed[0], size, false);
	SetMemory(&buttons.released[0], size, false);
	SetMemory(&buttons.down[0], size, false);
	return buttons;
}

void InitializeInput()
{
	input.mouse.buttons = NewInputButtons(MouseButtonCount);
	input.keyboard = NewInputButtons(ScancodeCount);
}

void PressButton(InputButtons *b, s64 i)
{
	b->pressed[i] = !b->down[i];
	b->down[i] = true;
}

void ReleaseButton(InputButtons *b, s64 i)
{
	b->released[i] = b->down[i];
	b->down[i] = false;
}

bool IsButtonDown(InputButtons *b, s64 i)
{
	return b->down[i];
}

bool WasButtonPressed(InputButtons *b, s64 i)
{
	return b->pressed[i];
}

bool WasButtonReleased(InputButtons *b, s64 i)
{
	return b->released[i];
}

bool IsKeyDown(KeySymbol k)
{
	auto c = KeySymbolToScancode(k);
	if (c == 0)
	{
		return false;
	}
	return IsButtonDown(&input.keyboard, c);
}

bool WasKeyPressed(KeySymbol k)
{
	auto c = KeySymbolToScancode(k);
	if (c == 0)
	{
		return false;
	}
	return WasButtonPressed(&input.keyboard, c);
}

bool WasKeyReleased(KeySymbol k)
{
	auto c = KeySymbolToScancode(k);
	if (c == 0)
	{
		return false;
	}
	return WasButtonReleased(&input.keyboard, c);
}

bool IsMouseButtonDown(MouseButton b)
{
	if (b < 0 || b >= MouseButtonCount)
	{
		return false;
	}
	return IsButtonDown(&input.mouse.buttons, b);
}

bool WasMouseButtonPressed(MouseButton b)
{
	if (b < 0 || b >= MouseButtonCount)
	{
		return false;
	}
	return WasButtonPressed(&input.mouse.buttons, b);
}

bool WasMouseButtonReleased(MouseButton b)
{
	if (b < 0 || b >= MouseButtonCount)
	{
		return false;
	}
	return WasButtonReleased(&input.mouse.buttons, b);
}

f32 MouseX()
{
	return input.mouse.x;
}

f32 MouseY()
{
	return input.mouse.y;
}

f32 MouseDeltaX()
{
	return input.mouse.rawDeltaX;
}

f32 MouseDeltaY()
{
	return input.mouse.rawDeltaY;
}

f32 MouseSensitivity()
{
	return input.mouse.sensitivity;
}

bool QuitWindowEvent()
{
	return input.windowEvents.quit;
}

WindowEvents GetInput(PlatformWindow *w)
{
	// Clear per-frame input.
	SetMemory(&input.mouse.buttons.pressed[0], sizeof(bool) * MouseButtonCount, false);
	SetMemory(&input.mouse.buttons.released[0], sizeof(bool) * MouseButtonCount, false);
	SetMemory(&input.keyboard.pressed[0], sizeof(bool) * ScancodeCount, false);
	SetMemory(&input.keyboard.released[0], sizeof(bool) * ScancodeCount, false);
	input.mouse.rawDeltaX = 0;
	input.mouse.rawDeltaY = 0;

	auto windowEvents = ProcessWindowEvents(w, &input.keyboard, &input.mouse);

	// Update mouse position.
	auto oldX = input.mouse.x;
	auto oldY = input.mouse.y;
	QueryMousePosition(w, &input.mouse.x, &input.mouse.y);
	input.mouse.deltaX = input.mouse.x - oldX;
	input.mouse.deltaY = input.mouse.y - oldY;

	return windowEvents;
}
