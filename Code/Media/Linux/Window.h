#pragma once

#include "../PCH.h"

#include "Code/Common.h"

struct PlatformWindow
{
	Window x11Window;
	Atom x11DeleteWindowAtom;
	Cursor x11BlankCursor;
	u32 height;
};

struct WindowEvents
{
	bool quit = false;
};

struct InputButtons;
struct Mouse;

void InitializeWindow(bool multithreaded);

Display *GetX11Display();

PlatformWindow CreateWindow(s64 width, s64 height, bool startFullscreen);
WindowEvents ProcessWindowEvents(PlatformWindow *window, InputButtons *keyboard, Mouse *mouse);
void ToggleFullscreen(PlatformWindow *window);
void CaptureCursor(PlatformWindow *window);
void UncaptureCursor(PlatformWindow *window);
void DestroyWindow(PlatformWindow *window);

#if defined(USE_VULKAN_RENDER_API)
	const char *GetRequiredVulkanSurfaceInstanceExtension();
	VkResult CreateVulkanSurface(PlatformWindow *window, VkInstance instance, VkSurfaceKHR *surface);
#endif
