#pragma once

#include "Basic/String.h"

struct VulkanSPIRV
{
	Array<Array<u8>> stageByteCode;
	Array<VkShaderStageFlagBits> stages;
};

VulkanSPIRV CompileGPUShaderToSPIRV(String filename, bool *err);
