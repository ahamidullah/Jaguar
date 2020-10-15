#pragma once

#include "Media/PCH.h"
#include "Basic/PCH.h"

#ifdef VulkanBuild
	#define VK_NO_PROTOTYPES
	#define VK_USE_PLATFORM_XCB_KHR
	#include "vulkan/vulkan.h"
#endif

// Asset
void *AllocateMemory(long size);
void *ResizeMemory(void *mem, long newSize);
void DeallocateMemory(void *mem);
#define STBI_MALLOC AllocateMemory
#define STBI_REALLOC ResizeMemory
#define STBI_FREE DeallocateMemory
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION
#undef STB_IMAGE_WRITE_IMPLEMENTATION

// Math
#include <math.h>
#include <float.h>
