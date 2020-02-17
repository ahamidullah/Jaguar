#include "Common/Memory.h"

#if defined(__linux__)
	#include "Platform/Linux/Windows.cpp"
#else
	#error unsupported platform
#endif

template <size_t Size>
void PlatformPressButton(u32 buttonIndex, PlatformInputButtons<Size> *buttons) {
	buttons->pressed[buttonIndex] = !buttons->down[buttonIndex];
	buttons->down[buttonIndex] = true;
}

template <size_t Size>
void PlatformReleaseButton(u32 buttonIndex, PlatformInputButtons<Size> *buttons) {
	buttons->released[buttonIndex] = buttons->down[buttonIndex];
	buttons->down[buttonIndex] = false;
}

template <size_t Size>
bool PlatformIsButtonDown(u32 buttonIndex, PlatformInputButtons<Size> *buttons) {
	return buttons->down[buttonIndex];
}

template <size_t Size>
bool PlatformWasButtonPressed(u32 buttonIndex, PlatformInputButtons<Size> *buttons) {
	return buttons->pressed[buttonIndex];
}

template <size_t Size>
bool PlatformWasButtonReleased(u32 buttonIndex, PlatformInputButtons<Size> *buttons) {
	return buttons->released[buttonIndex];
}

bool PlatformIsKeyDown(PlatformKeySymbol keySymbol, PlatformInput *input) {
	s32 keyCode = PlatformKeySymbolToScancode(keySymbol);
	if (keyCode == 0) {
		return false;
	}
	return PlatformIsButtonDown(keyCode, &input->keyboard);
}

bool PlatformWasKeyPressed(PlatformKeySymbol keySymbol, PlatformInput *input) {
	s32 keyCode = PlatformKeySymbolToScancode(keySymbol);
	if (keyCode == 0) {
		return false;
	}
	return PlatformWasButtonPressed(keyCode, &input->keyboard);
}

bool PlatformWasKeyReleased(PlatformKeySymbol keySymbol, PlatformInput *input) {
	s32 keyCode = PlatformKeySymbolToScancode(keySymbol);
	if (keyCode == 0) {
		return false;
	}
	return PlatformWasButtonReleased(keyCode, &input->keyboard);
}

bool PlatformIsMouseButtonDown(PlatformMouseButton mouseButton, PlatformInput *input) {
	if (mouseButton < 0 || mouseButton >= MOUSE_BUTTON_COUNT) {
		return false;
	}
	return PlatformIsButtonDown(mouseButton, &input->mouse.buttons);
}

bool PlatformWasMouseButtonPressed(PlatformMouseButton mouseButton, PlatformInput *input) {
	if (mouseButton < 0 || mouseButton >= MOUSE_BUTTON_COUNT) {
		return false;
	}
	return PlatformWasButtonPressed(mouseButton, &input->mouse.buttons);
}

bool PlatformWasMouseButtonReleased(PlatformMouseButton mouseButton, PlatformInput *input) {
	if (mouseButton < 0 || mouseButton >= MOUSE_BUTTON_COUNT) {
		return false;
	}
	return PlatformWasButtonReleased(mouseButton, &input->mouse.buttons);
}

void PlatformGetInput(PlatformWindow window, PlatformInput *input, PlatformWindowEvents *windowEvents) {
	SetMemory(windowEvents, 0, sizeof(PlatformWindowEvents));
	SetMemory(input->mouse.buttons.pressed, false, sizeof(bool) * MOUSE_BUTTON_COUNT);
	SetMemory(input->mouse.buttons.released, false, sizeof(bool) * MOUSE_BUTTON_COUNT);
	SetMemory(input->keyboard.pressed, false, sizeof(bool) * SCANCODE_COUNT);
	SetMemory(input->keyboard.released, false, sizeof(bool) * SCANCODE_COUNT);
	input->mouse.rawDeltaX = 0;
	input->mouse.rawDeltaY = 0;

	PlatformProcessEvents(window, windowEvents, input);

	s32 oldX = input->mouse.x;
	s32 oldY = input->mouse.y;
	PlatformGetMousePosition(window, &input->mouse.x, &input->mouse.y);
	input->mouse.deltaX = input->mouse.x - oldX;
	input->mouse.deltaY = input->mouse.y - oldY;
}
