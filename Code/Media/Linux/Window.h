#pragma once

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

struct WindowContext
{
	Window x11;
	Atom deleteWindowAtom;
	Cursor blankCursor;
	u32 height;
};

struct Input;

WindowContext CreateWindow(s32 width, s32 height, bool startFullscreen);
void ProcessWindowEvents(WindowContext *window, Input *input);
void ToggleFullscreen(WindowContext *window);
void CaptureCursor(WindowContext *window);
void UncaptureCursor(WindowContext *window);
void DestroyWindow(WindowContext *window);
void InitializeWindow();

#if defined(USE_VULKAN_RENDER_API)

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_xlib.h> 

const char *GetRequiredVulkanSurfaceInstanceExtension();
VkResult CreateVulkanSurface(WindowContext *window, VkInstance instance, VkSurfaceKHR *surface);

#endif
