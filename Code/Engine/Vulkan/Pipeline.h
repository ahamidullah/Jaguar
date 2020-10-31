#pragma once

#ifdef VulkanBuild

namespace GPU
{

VkPipeline NewPipeline(String shaderFilename, ArrayView<VkShaderStageFlagBits> stages, ArrayView<VkShaderModule> modules, VkRenderPass rp);

}

#endif
