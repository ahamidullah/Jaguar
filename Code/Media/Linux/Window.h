#pragma once

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

struct PlatformWindow
{
	Window x11Window;
	Atom x11DeleteWindowAtom;
	Cursor x11BlankCursor;
	u32 height;
};

struct InputButtons;
struct Mouse;
struct WindowEvents;

PlatformWindow CreateWindow(s32 width, s32 height, bool startFullscreen = false);
void ProcessWindowEvents(PlatformWindow *window, InputButtons *keyboard, Mouse *mouse, WindowEvents *windowEvents);
void ToggleFullscreen(PlatformWindow *window);
void CaptureCursor(PlatformWindow *window);
void UncaptureCursor(PlatformWindow *window);
void DestroyWindow(PlatformWindow *window);
void InitializeWindow();

#if defined(USE_VULKAN_RENDER_API)

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_xlib.h> 

const char *GetRequiredVulkanSurfaceInstanceExtension();
VkResult CreateVulkanSurface(PlatformWindow *window, VkInstance instance, VkSurfaceKHR *surface);

#endif
