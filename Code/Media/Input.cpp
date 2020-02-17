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

bool IsKeyDown(KeySymbol keySymbol, Input *input)
{
	s32 keyCode = KeySymbolToScancode(keySymbol);
	if (keyCode == 0)
	{
		return false;
	}
	return IsButtonDown(keyCode, &input->keyboard);
}

bool WasKeyPressed(KeySymbol keySymbol, Input *input)
{
	s32 keyCode = KeySymbolToScancode(keySymbol);
	if (keyCode == 0)
	{
		return false;
	}
	return WasButtonPressed(keyCode, &input->keyboard);
}

bool WasKeyReleased(KeySymbol keySymbol, Input *input)
{
	s32 keyCode = KeySymbolToScancode(keySymbol);
	if (keyCode == 0)
	{
		return false;
	}
	return WasButtonReleased(keyCode, &input->keyboard);
}

bool IsMouseButtonDown(MouseButton mouseButton, Input *input)
{
	if (mouseButton < 0 || mouseButton >= MOUSE_BUTTON_COUNT)
	{
		return false;
	}
	return IsButtonDown(mouseButton, &input->mouse.buttons);
}

bool WasMouseButtonPressed(MouseButton mouseButton, Input *input)
{
	if (mouseButton < 0 || mouseButton >= MOUSE_BUTTON_COUNT)
	{
		return false;
	}
	return WasButtonPressed(mouseButton, &input->mouse.buttons);
}

bool WasMouseButtonReleased(MouseButton mouseButton, Input *input)
{
	if (mouseButton < 0 || mouseButton >= MOUSE_BUTTON_COUNT)
	{
		return false;
	}
	return WasButtonReleased(mouseButton, &input->mouse.buttons);
}

void GetPlatformInput(WindowContext *window, Input *input)
{
	// Clear per-frame input.
	SetMemory(input->mouse.buttons.pressed, false, sizeof(bool) * MOUSE_BUTTON_COUNT);
	SetMemory(input->mouse.buttons.released, false, sizeof(bool) * MOUSE_BUTTON_COUNT);
	SetMemory(input->keyboard.pressed, false, sizeof(bool) * SCANCODE_COUNT);
	SetMemory(input->keyboard.released, false, sizeof(bool) * SCANCODE_COUNT);
	input->mouse.rawDeltaX = 0;
	input->mouse.rawDeltaY = 0;

	ProcessWindowEvents(window, input);

	// Update mouse position.
	s32 oldX = input->mouse.x;
	s32 oldY = input->mouse.y;
	GetMousePosition(window, &input->mouse.x, &input->mouse.y);
	input->mouse.deltaX = input->mouse.x - oldX;
	input->mouse.deltaY = input->mouse.y - oldY;
}
