#pragma once

#ifdef VulkanBuild

namespace GPU::Vulkan
{

VkPipelineLayout NewPipelineLayout(Device d);
VkPipeline NewPipeline(Device d, VkPipelineLayout l, String shaderFilename, array::View<VkShaderStageFlagBits> stages, array::View<VkShaderModule> modules, VkRenderPass rp);

}

#endif
