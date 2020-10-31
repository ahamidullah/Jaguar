#ifdef VulkanBuild

#include "Shader.h"
#include "Pipeline.h"
#include "RenderPass.h"

namespace GPU
{

Shader CompileShader(String filename, bool *err)
{
	auto spirv = ShaderCompiler::VulkanGLSL(filename, err);
	if (*err)
	{
		LogError("Vulkan", "Failed to generate SPIRV for file %k.", filename);
		return {};
	}
	auto s = GPU::Shader
	{
		.vkStages = spirv.stages,
	};
	for (auto bc : spirv.stageByteCode)
	{
		auto ci = VkShaderModuleCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = (u32)bc.count,
			.pCode = (u32 *)bc.elements,
		};
		auto m = VkShaderModule{};
		VkCheck(vkCreateShaderModule(vkDevice, &ci, NULL, &m));
		s.vkModules.Append(m);
	}
	s.vkRenderPass = NewRenderPass(filename);
	s.vkPipeline = NewPipeline(filename, s.vkStages, s.vkModules, s.vkRenderPass);
	return s;
}

}

#endif

