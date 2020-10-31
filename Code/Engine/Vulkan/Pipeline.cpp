#ifdef VulkanBuild

namespace GPU
{

// @TODO: Pipeline cache.

VkPipeline NewPipeline(String shaderFilename, ArrayView<VkShaderStageFlagBits> stages, ArrayView<VkShaderModule> modules, VkRenderPass rp)
{
	if (shaderFilename == "Model.glsl")
	{
		auto assemblyCI = VkPipelineInputAssemblyStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = VK_FALSE,
		};
		auto viewport = VkViewport
		{
			.x = 0.0f,
			.y = 0.0f,
			.width = (f32)RenderWidth(),
			.height = (f32)RenderHeight(),
			.minDepth = 0.0f,
			.maxDepth = 1.0f,
		};
		auto scissor = VkRect2D
		{
			.offset = {0, 0},
			.extent = (VkExtent2D){(u32)RenderWidth(), (u32)RenderHeight()},
		};
		auto viewportCI = VkPipelineViewportStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1,
			.pViewports = &viewport,
			.scissorCount = 1,
			.pScissors = &scissor,
		};
		auto rasterCI = VkPipelineRasterizationStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_BACK_BIT,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable = false,
			.lineWidth = 1.0f,
		};
		auto multisampleCI = VkPipelineMultisampleStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			.sampleShadingEnable = VK_FALSE,
		};
		auto depthCI = VkPipelineDepthStencilStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthTestEnable = VK_TRUE,
			.depthWriteEnable = VK_TRUE,
			.depthCompareOp = VK_COMPARE_OP_LESS,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE,
		};
		auto blendStates = MakeStaticArray<VkPipelineColorBlendAttachmentState>(
			{
				.blendEnable = true,
				.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
				.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
				.colorBlendOp = VK_BLEND_OP_ADD,
				.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
				.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
				.alphaBlendOp = VK_BLEND_OP_ADD,
				.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			});
		auto blendCI = VkPipelineColorBlendStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_COPY,
			.attachmentCount = (u32)blendStates.Count(),
			.pAttachments = blendStates.elements,
			.blendConstants = {},
		};
		auto vertexCI = VkPipelineVertexInputStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		};
		auto dynStates = MakeStaticArray<VkDynamicState>(
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR);
		auto dynStateCI = VkPipelineDynamicStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = (u32)dynStates.Count(),
			.pDynamicStates = dynStates.elements,
		};
		auto stageCIs = Array<VkPipelineShaderStageCreateInfo>{};
		for (auto i = 0; i < modules.count; i += 1)
		{
			stageCIs.Append(
				{
					.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
					.stage = stages[i],
					.module = modules[i],
					.pName = "main",
				}
			);
		}
		auto ci = VkGraphicsPipelineCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.stageCount = (u32)stageCIs.count,
			.pStages = stageCIs.elements,
			.pVertexInputState = &vertexCI,
			.pInputAssemblyState = &assemblyCI,
			.pViewportState = &viewportCI,
			.pRasterizationState = &rasterCI,
			.pMultisampleState = &multisampleCI,
			.pDepthStencilState = &depthCI,
			.pColorBlendState = &blendCI,
			.pDynamicState = &dynStateCI,
			.layout = vkPipelineLayout,
			.renderPass = rp,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE,
			.basePipelineIndex = -1,
		};
		auto p = VkPipeline{};
		VkCheck(vkCreateGraphicsPipelines(vkDevice, vkPipelineCache, 1, &ci, NULL, &p));
		return p;
	}
	else
	{
		Abort("Vulkan", "Failed to make pipeline: unknown shader %k.", shaderFilename);
	}
	return VkPipeline{};
}

}

#endif
