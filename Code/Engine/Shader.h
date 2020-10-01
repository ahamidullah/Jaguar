#pragma once

#include "Basic/String.h"

struct VulkanSPIRVInfo
{
	String name;
	Array<String> filepaths;
	Array<VkShaderStageFlagBits> stages;
};

Array<String> GPUShaderFilepaths();
VulkanSPIRVInfo GenerateVulkanSPIRV(String filepath, bool *err);
