#include "Input.h"
#include "Basic/Memory.h"

struct InputButtons
{
	Array<bool> down;
	Array<bool> pressed;
	Array<bool> released;

	void Press(s64 i);
	void Release(s64 i);
	bool IsDown(s64 i);
	bool WasPressed(s64 i);
	bool WasReleased(s64 i);
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

struct Input
{
	Mouse mouse;
	InputButtons keyboard;
	WindowEvents windowEvents;
} input;

InputButtons NewInputButtons(s64 size)
{
	auto b = InputButtons
	{
		.down = NewArrayIn<bool>(GlobalHeap(), size),
		.pressed = NewArrayIn<bool>(GlobalHeap(), size),
		.released = NewArrayIn<bool>(GlobalHeap(), size),
	};
	for (auto &p : b.pressed)
	{
		p = false;
	}
	for (auto &r : b.released)
	{
		r = false;
	}
	for (auto &d : b.down)
	{
		d = false;
	}
	return b;
}

void InputButtons::Press(s64 i)
{
	this->pressed[i] = !this->down[i];
	this->down[i] = true;
}

void InputButtons::Release(s64 i)
{
	this->released[i] = this->down[i];
	this->down[i] = false;
}

bool InputButtons::IsDown(s64 i)
{
	return this->down[i];
}

bool InputButtons::WasPressed(s64 i)
{
	return this->pressed[i];
}

bool InputButtons::WasReleased(s64 i)
{
	return this->released[i];
}

void InitializeInput()
{
	input.mouse.buttons = NewInputButtons(MouseButtonCount);
	input.keyboard = NewInputButtons(ScancodeCount);
	InitializePlatformInput();
}

bool IsKeyDown(KeySymbol k)
{
	auto c = KeySymbolToScancode(k);
	if (c == 0)
	{
		return false;
	}
	return input.keyboard.IsDown(c);
}

bool WasKeyPressed(KeySymbol k)
{
	auto c = KeySymbolToScancode(k);
	if (c == 0)
	{
		return false;
	}
	return input.keyboard.WasPressed(c);
}

bool WasKeyReleased(KeySymbol k)
{
	auto c = KeySymbolToScancode(k);
	if (c == 0)
	{
		return false;
	}
	return input.keyboard.WasReleased(c);
}

bool IsMouseButtonDown(MouseButton b)
{
	if (b < 0 || b >= MouseButtonCount)
	{
		return false;
	}
	return input.keyboard.IsDown(b);
}

bool WasMouseButtonPressed(MouseButton b)
{
	if (b < 0 || b >= MouseButtonCount)
	{
		return false;
	}
	return input.keyboard.WasPressed(b);
}

bool WasMouseButtonReleased(MouseButton b)
{
	if (b < 0 || b >= MouseButtonCount)
	{
		return false;
	}
	return input.keyboard.WasReleased(b);
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
	for (auto &p : input.mouse.buttons.pressed)
	{
		p = false;
	}
	for (auto &r : input.mouse.buttons.released)
	{
		r = false;
	}
	for (auto &p : input.keyboard.pressed)
	{
		p = false;
	}
	for (auto &r : input.keyboard.released)
	{
		r = false;
	}
	input.mouse.rawDeltaX = 0;
	input.mouse.rawDeltaY = 0;
	auto wev = ProcessWindowEvents(w, &input.keyboard, &input.mouse);
	// Update mouse position.
	auto oldX = input.mouse.x;
	auto oldY = input.mouse.y;
	QueryMousePosition(w, &input.mouse.x, &input.mouse.y);
	input.mouse.deltaX = input.mouse.x - oldX;
	input.mouse.deltaY = input.mouse.y - oldY;
	return wev;
}
