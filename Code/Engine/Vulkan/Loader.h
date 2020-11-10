#pragma once

#ifdef VulkanBuild

#include "Instance.h"
#include "Device.h"
#include "Basic/DLL.h"

namespace GPU::Vulkan
{

void LoadExportedFunctions(DLL d);
void LoadGlobalFunctions();
void LoadInstanceFunctions(Instance i);
void LoadDeviceFunctions(Device d);

}

#endif
