#include "Input.h"
#include "Event.h"
#include "Basic/Memory.h"

struct InputButtons
{
	Array<bool> down;
	Array<bool> pressed;
	Array<bool> released;

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

InputButtons NewInputButtons(s64 size)
{
	auto b = InputButtons
	{
		.down = NewArrayIn<bool>(GlobalAllocator(), size),
		.pressed = NewArrayIn<bool>(GlobalAllocator(), size),
		.released = NewArrayIn<bool>(GlobalAllocator(), size),
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

bool InputButtons::Down(s64 i)
{
	return this->down[i];
}

bool InputButtons::Pressed(s64 i)
{
	return this->pressed[i];
}

bool InputButtons::Released(s64 i)
{
	return this->released[i];
}

struct Input
{
	Mouse mouse;
	InputButtons keyboard;
} input;

Input NewInput()
{
	return
	{
		.mouse.buttons = NewInputButtons(MouseButtonCount),
		.keyboard = NewInputButtons(ScancodeCount),
	};
}

void InitializeInput()
{
	InitializePlatformInput();
	input = NewInput();
}

bool KeyDown(KeySymbol k)
{
	auto c = KeySymbolToScancode(k);
	if (c == 0)
	{
		return false;
	}
	return input.keyboard.Down(c);
}

bool KeyPressed(KeySymbol k)
{
	auto c = KeySymbolToScancode(k);
	if (c == 0)
	{
		return false;
	}
	return input.keyboard.Pressed(c);
}

bool KeyReleased(KeySymbol k)
{
	auto c = KeySymbolToScancode(k);
	if (c == 0)
	{
		return false;
	}
	return input.keyboard.Released(c);
}

bool MouseButtonDown(MouseButton b)
{
	if (b < 0 || b >= MouseButtonCount)
	{
		return false;
	}
	return input.keyboard.Down(b);
}

bool MouseButtonPressed(MouseButton b)
{
	if (b < 0 || b >= MouseButtonCount)
	{
		return false;
	}
	return input.keyboard.Pressed(b);
}

bool MouseButtonReleased(MouseButton b)
{
	if (b < 0 || b >= MouseButtonCount)
	{
		return false;
	}
	return input.keyboard.Released(b);
}

s32 MouseX()
{
	return input.mouse.x;
}

s32 MouseY()
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

PlatformEvents ProcessInput(Window *w)
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
	auto wev = ProcessPlatformEvents(w, &input.keyboard, &input.mouse);
	// Update mouse position.
	auto oldX = input.mouse.x;
	auto oldY = input.mouse.y;
	QueryMousePosition(w, &input.mouse.x, &input.mouse.y);
	input.mouse.deltaX = input.mouse.x - oldX;
	input.mouse.deltaY = input.mouse.y - oldY;
	return wev;
}
