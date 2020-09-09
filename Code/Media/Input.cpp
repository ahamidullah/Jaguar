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

struct InputState
{
	Mouse mouse;
	InputButtons keyboard;
};

InputState NewInputState()
{
	return
	{
		.mouse.buttons = NewInputButtons(MouseButtonCount),
		.keyboard = NewInputButtons(ScancodeCount),
	};
}

InputState *Input()
{
	static auto i = NewInputState();
	return &i;
}

bool IsKeyDown(KeySymbol k)
{
	auto c = KeySymbolToScancode(k);
	if (c == 0)
	{
		return false;
	}
	return Input()->keyboard.IsDown(c);
}

bool WasKeyPressed(KeySymbol k)
{
	auto c = KeySymbolToScancode(k);
	if (c == 0)
	{
		return false;
	}
	return Input()->keyboard.WasPressed(c);
}

bool WasKeyReleased(KeySymbol k)
{
	auto c = KeySymbolToScancode(k);
	if (c == 0)
	{
		return false;
	}
	return Input()->keyboard.WasReleased(c);
}

bool IsMouseButtonDown(MouseButton b)
{
	if (b < 0 || b >= MouseButtonCount)
	{
		return false;
	}
	return Input()->keyboard.IsDown(b);
}

bool WasMouseButtonPressed(MouseButton b)
{
	if (b < 0 || b >= MouseButtonCount)
	{
		return false;
	}
	return Input()->keyboard.WasPressed(b);
}

bool WasMouseButtonReleased(MouseButton b)
{
	if (b < 0 || b >= MouseButtonCount)
	{
		return false;
	}
	return Input()->keyboard.WasReleased(b);
}

f32 MouseX()
{
	return Input()->mouse.x;
}

f32 MouseY()
{
	return Input()->mouse.y;
}

f32 MouseDeltaX()
{
	return Input()->mouse.rawDeltaX;
}

f32 MouseDeltaY()
{
	return Input()->mouse.rawDeltaY;
}

f32 MouseSensitivity()
{
	return Input()->mouse.sensitivity;
}

WindowEvents ProcessInput(PlatformWindow *w)
{
	// Clear per-frame input.
	for (auto &p : Input()->mouse.buttons.pressed)
	{
		p = false;
	}
	for (auto &r : Input()->mouse.buttons.released)
	{
		r = false;
	}
	for (auto &p : Input()->keyboard.pressed)
	{
		p = false;
	}
	for (auto &r : Input()->keyboard.released)
	{
		r = false;
	}
	Input()->mouse.rawDeltaX = 0;
	Input()->mouse.rawDeltaY = 0;
	auto wev = ProcessWindowEvents(w, &Input()->keyboard, &Input()->mouse);
	// Update mouse position.
	auto oldX = Input()->mouse.x;
	auto oldY = Input()->mouse.y;
	QueryMousePosition(w, &Input()->mouse.x, &Input()->mouse.y);
	Input()->mouse.deltaX = Input()->mouse.x - oldX;
	Input()->mouse.deltaY = Input()->mouse.y - oldY;
	return wev;
}
