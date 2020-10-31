#pragma once

#ifdef VulkanBuild

namespace GPU
{

struct Shader
{
	Array<VkShaderStageFlagBits> vkStages;
	Array<VkShaderModule> vkModules;
	VkRenderPass vkRenderPass;
	VkPipeline vkPipeline;
};

Shader CompileShader(String filename, bool *err);

}

#endif
