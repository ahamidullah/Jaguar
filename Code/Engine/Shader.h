#pragma once

#include "Basic/String.h"
#include "Basic/Container/Array.h"

namespace ShaderCompiler
{

#ifdef VulkanBuild

struct SPIRV
{
	array::Array<array::Array<u8>> stageByteCode;
	array::Array<VkShaderStageFlagBits> stages;
};

SPIRV VulkanGLSL(String filename, bool *err);

#endif

}
