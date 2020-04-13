static struct
{
	Mouse mouse;
	InputButtons keyboard =
	{
		AllocateArray(bool, SCANCODE_COUNT),
		AllocateArray(bool, SCANCODE_COUNT),
		AllocateArray(bool, SCANCODE_COUNT),
	};
	WindowEvents windowEvents;
} inputState;

void PressButton(u32 buttonIndex, InputButtons *buttons)
{
	buttons->pressed[buttonIndex] = !buttons->down[buttonIndex];
	buttons->down[buttonIndex] = true;
}

void ReleaseButton(u32 buttonIndex, InputButtons *buttons)
{
	buttons->released[buttonIndex] = buttons->down[buttonIndex];
	buttons->down[buttonIndex] = false;
}

bool IsButtonDown(u32 buttonIndex, InputButtons *buttons)
{
	return buttons->down[buttonIndex];
}

bool WasButtonPressed(u32 buttonIndex, InputButtons *buttons)
{
	return buttons->pressed[buttonIndex];
}

bool WasButtonReleased(u32 buttonIndex, InputButtons *buttons)
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
	return IsButtonDown(keyCode, &inputState.keyboard);
}

bool WasKeyPressed(KeySymbol keySymbol)
{
	s32 keyCode = KeySymbolToScancode(keySymbol);
	if (keyCode == 0)
	{
		return false;
	}
	return WasButtonPressed(keyCode, &inputState.keyboard);
}

bool WasKeyReleased(KeySymbol keySymbol)
{
	s32 keyCode = KeySymbolToScancode(keySymbol);
	if (keyCode == 0)
	{
		return false;
	}
	return WasButtonReleased(keyCode, &inputState.keyboard);
}

bool IsMouseButtonDown(MouseButton mouseButton)
{
	if (mouseButton < 0 || mouseButton >= MOUSE_BUTTON_COUNT)
	{
		return false;
	}
	return IsButtonDown(mouseButton, &inputState.mouse.buttons);
}

bool WasMouseButtonPressed(MouseButton mouseButton)
{
	if (mouseButton < 0 || mouseButton >= MOUSE_BUTTON_COUNT)
	{
		return false;
	}
	return WasButtonPressed(mouseButton, &inputState.mouse.buttons);
}

bool WasMouseButtonReleased(MouseButton mouseButton)
{
	if (mouseButton < 0 || mouseButton >= MOUSE_BUTTON_COUNT)
	{
		return false;
	}
	return WasButtonReleased(mouseButton, &inputState.mouse.buttons);
}

f32 GetMouseX()
{
	return inputState.mouse.x;
}

f32 GetMouseY()
{
	return inputState.mouse.y;
}

f32 GetMouseDeltaX()
{
	return inputState.mouse.rawDeltaX;
}

f32 GetMouseDeltaY()
{
	return inputState.mouse.rawDeltaY;
}

f32 GetMouseSensitivity()
{
	return inputState.mouse.sensitivity;
}

bool QuitWindowEvent()
{
	return inputState.windowEvents.quit;
}

void GetPlatformInput(WindowContext *window)
{
	// Clear per-frame input.
	SetMemory(inputState.mouse.buttons.pressed, false, sizeof(bool) * MOUSE_BUTTON_COUNT);
	SetMemory(inputState.mouse.buttons.released, false, sizeof(bool) * MOUSE_BUTTON_COUNT);
	SetMemory(inputState.keyboard.pressed, false, sizeof(bool) * SCANCODE_COUNT);
	SetMemory(inputState.keyboard.released, false, sizeof(bool) * SCANCODE_COUNT);
	inputState.mouse.rawDeltaX = 0;
	inputState.mouse.rawDeltaY = 0;

	ProcessWindowEvents(window, &inputState.keyboard, &inputState.mouse, &inputState.windowEvents);

	// Update mouse position.
	s32 oldX = inputState.mouse.x;
	s32 oldY = inputState.mouse.y;
	QueryMousePosition(window, &inputState.mouse.x, &inputState.mouse.y);
	inputState.mouse.deltaX = inputState.mouse.x - oldX;
	inputState.mouse.deltaY = inputState.mouse.y - oldY;
}
