#pragma once

#if defined(__linux__)
	#include "Platform/Linux/Windows.h"
#else
	#error unsupported platform
#endif

#define SCANCODE_COUNT 256

struct PlatformWindowEvents {
	bool quit;
};

template <size_t Size>
struct PlatformInputButtons {
	bool down[Size];
	bool pressed[Size];
	bool released[Size];
};

struct PlatformMouse {
	s32 wheel;
	s32 x, y;
	s32 deltaX, deltaY;
	f32 rawDeltaX, rawDeltaY;
	f32 sensitivity;
	PlatformInputButtons<MOUSE_BUTTON_COUNT> buttons;
};

struct PlatformInput {
	PlatformMouse mouse;
	PlatformInputButtons<SCANCODE_COUNT> keyboard;
	bool gotQuitEvent;
};

PlatformWindow PlatformCreateWindow(s32 windowWidth, s32 windowHeight, bool startFullscreen);
void PlatformToggleFullscreen();
void PlatformCaptureCursor();
void PlatformUncaptureCursor();
void PlatformCleanupDisplay();
void PlatformProcessWindowEvents(PlatformWindow window, PlatformWindowEvents *windowEvents, PlatformInput *input);

void PlatformGetInput(PlatformInput *input);

bool PlatformIsKeyDown(PlatformKeySymbol keySymbol, PlatformInput *input);
bool PlatformWasKeyPressed(PlatformKeySymbol keySymbol, PlatformInput *input);
bool PlatformWasKeyReleased(PlatformKeySymbol keySymbol, PlatformInput *input);

bool PlatformIsMouseButtonDown(PlatformMouseButton mouseButton, PlatformInput *input);
bool PlatformWasMouseButtonPressed(PlatformMouseButton mouseButton, PlatformInput *input);
bool PlatformWasMouseButtonReleased(PlatformMouseButton mouseButton, PlatformInput *input);

#if defined(USE_VULKAN_RENDER_API)

const char *PlatformGetRequiredVulkanSurfaceInstanceExtension();
VkResult PlatformCreateVulkanSurface(PlatformWindow window, VkInstance instance, VkSurfaceKHR *surface);

#endif
