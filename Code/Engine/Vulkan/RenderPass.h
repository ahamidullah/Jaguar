#pragma once

#ifdef VulkanBuild

namespace GPU::Vulkan
{

VkRenderPass NewRenderPass(Device d, VkSurfaceFormatKHR sf, string::String shaderFilename);

}

#endif

