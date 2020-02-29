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
} inputGlobals;

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
	return IsButtonDown(keyCode, &inputGlobals.keyboard);
}

bool WasKeyPressed(KeySymbol keySymbol)
{
	s32 keyCode = KeySymbolToScancode(keySymbol);
	if (keyCode == 0)
	{
		return false;
	}
	return WasButtonPressed(keyCode, &inputGlobals.keyboard);
}

bool WasKeyReleased(KeySymbol keySymbol)
{
	s32 keyCode = KeySymbolToScancode(keySymbol);
	if (keyCode == 0)
	{
		return false;
	}
	return WasButtonReleased(keyCode, &inputGlobals.keyboard);
}

bool IsMouseButtonDown(MouseButton mouseButton)
{
	if (mouseButton < 0 || mouseButton >= MOUSE_BUTTON_COUNT)
	{
		return false;
	}
	return IsButtonDown(mouseButton, &inputGlobals.mouse.buttons);
}

bool WasMouseButtonPressed(MouseButton mouseButton)
{
	if (mouseButton < 0 || mouseButton >= MOUSE_BUTTON_COUNT)
	{
		return false;
	}
	return WasButtonPressed(mouseButton, &inputGlobals.mouse.buttons);
}

bool WasMouseButtonReleased(MouseButton mouseButton)
{
	if (mouseButton < 0 || mouseButton >= MOUSE_BUTTON_COUNT)
	{
		return false;
	}
	return WasButtonReleased(mouseButton, &inputGlobals.mouse.buttons);
}

f32 GetMouseX()
{
	return inputGlobals.mouse.x;
}

f32 GetMouseY()
{
	return inputGlobals.mouse.y;
}

f32 GetMouseDeltaX()
{
	return inputGlobals.mouse.rawDeltaX;
}

f32 GetMouseDeltaY()
{
	return inputGlobals.mouse.rawDeltaY;
}

f32 GetMouseSensitivity()
{
	return inputGlobals.mouse.sensitivity;
}

bool QuitWindowEvent()
{
	return inputGlobals.windowEvents.quit;
}

void GetPlatformInput(WindowContext *window)
{
	// Clear per-frame input.
	SetMemory(inputGlobals.mouse.buttons.pressed, false, sizeof(bool) * MOUSE_BUTTON_COUNT);
	SetMemory(inputGlobals.mouse.buttons.released, false, sizeof(bool) * MOUSE_BUTTON_COUNT);
	SetMemory(inputGlobals.keyboard.pressed, false, sizeof(bool) * SCANCODE_COUNT);
	SetMemory(inputGlobals.keyboard.released, false, sizeof(bool) * SCANCODE_COUNT);
	inputGlobals.mouse.rawDeltaX = 0;
	inputGlobals.mouse.rawDeltaY = 0;

	ProcessWindowEvents(window, &inputGlobals.keyboard, &inputGlobals.mouse, &inputGlobals.windowEvents);

	// Update mouse position.
	s32 oldX = inputGlobals.mouse.x;
	s32 oldY = inputGlobals.mouse.y;
	QueryMousePosition(window, &inputGlobals.mouse.x, &inputGlobals.mouse.y);
	inputGlobals.mouse.deltaX = inputGlobals.mouse.x - oldX;
	inputGlobals.mouse.deltaY = inputGlobals.mouse.y - oldY;
}
