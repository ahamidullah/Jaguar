#pragma once

#include "Basic/String.h"

namespace ShaderCompiler
{

#ifdef VulkanBuild

struct SPIRV
{
	Array<Array<u8>> stageByteCode;
	Array<VkShaderStageFlagBits> stages;
};

SPIRV VulkanGLSL(String filename, bool *err);

#endif

}
