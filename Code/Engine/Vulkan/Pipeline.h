#pragma once

#ifdef VulkanBuild

namespace GPU::Vulkan
{

VkPipelineLayout NewPipelineLayout(Device d);
VkPipeline NewPipeline(Device d, VkPipelineLayout l, String shaderFilename, ArrayView<VkShaderStageFlagBits> stages, ArrayView<VkShaderModule> modules, VkRenderPass rp);

}

#endif
