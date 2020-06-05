#pragma once

#include "Code/Media/PCH.h"

#include "Code/Basic/PCH.h"

// Vulkan
#define VK_NO_PROTOTYPES
#define VK_USE_PLATFORM_XLIB_KHR
#include "vulkan/vulkan.h"

// Asset
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Math
#include <math.h>
#include <float.h>
