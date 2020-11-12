#pragma once

#ifdef VulkanBuild

namespace GPU::Vulkan
{

struct Shader
{
	array::Array<VkShaderStageFlagBits> vkStages;
	array::Array<VkShaderModule> vkModules;
	VkRenderPass vkRenderPass;
	VkPipeline vkPipeline;
};

Shader CompileShader(Device d, VkSurfaceFormatKHR sf, VkPipelineLayout l, string::String filename, bool *err);

}

#endif
